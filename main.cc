/**
 * piste: system/bt/hal/bluetooth_interface.cc (suivi notre ami bluetoothtbd->main->daemon.cc->bluetooth_interface.cc)
 * 
 * build: mettre dans un dir dans system/bt que tu ajoutes à Android.bp->subdirs
 * 
 * 
 * /etc/bluetooth/bt_stack.conf gère le log, par élément/librairie (btif, stack, ... u name it exemple: BTIF_TRACE_DEBUG)
 * 
adb push ./out/target/product/mido/system/bin/bt_vvnx /system/bin
 * 
 * hardware/libhardware/include/hardware/bluetooth.h
 * 
*/

#include <stdint.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>

#include "btcore/include/hal_util.h"
#include "stack/include/btm_ble_api.h"
#include "stack/include/btm_ble_api_types.h"


#include <hardware/bluetooth.h>
#include <hardware/hardware.h>
#include <hardware/bt_gatt.h>
#include <hardware/ble_scanner.h>

//external/libchrome/base/
#include <base/message_loop/message_loop.h>
#include <base/run_loop.h>
#include <base/callback.h>
#include <base/logging.h>
#include "base/bind.h"



void AdapterStateChangedCallback(bt_state_t state) {
  printf("VVNX Adapter state changed \n");
}

void device_found_cb(int num_properties, bt_property_t *properties) {
  printf("device found vvnx %i %i \n", num_properties, properties->len);
}

void ThreadEventCallback(bt_cb_thread_evt evt) {
}


// The HAL Bluetooth DM callbacks.
bt_callbacks_t bt_callbacks = {
    sizeof(bt_callbacks_t),
    &AdapterStateChangedCallback,
    nullptr,
    nullptr,
    &device_found_cb, /* device_found_cb */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &ThreadEventCallback,
    nullptr, /* dut_mode_recv_cb */
    nullptr, /* le_test_mode_cb */
    nullptr  /* energy_info_cb */
};

void scan_result_cb(uint16_t event_type, uint8_t addr_type,
					 RawAddress *bda, uint8_t primary_phy,
					 uint8_t secondary_phy,
					 uint8_t advertising_sid, int8_t tx_power,
					 int8_t rssi, uint16_t periodic_adv_int,
					 std::vector<uint8_t> adv_data) {
		std::string addrstr = bda->ToString();				 	
		printf("VVNX scan result cb bdaddr=%s addr_type=%i adv_sid=%i rssi=%i\n", addrstr.c_str(), addr_type, advertising_sid, rssi);		
}

//hardware/ble_scanner.h
btgatt_scanner_callbacks_t btgatt_scanner_callbacks = {
	&scan_result_cb,
    nullptr,
    nullptr,
    nullptr	
};

//hardware/bt_gatt.h
btgatt_callbacks_t bt_gatt_callbacks = {
	sizeof(btgatt_callbacks_t),
	nullptr,
    nullptr,
    &btgatt_scanner_callbacks
};


/**les callbacks de BleScannerInterface: un grand moment de solitude!
un grand merci à https://www.chromium.org/chromium-os/packages/libchromeos pour la syntaxe de ces callbacks à binder**/

/* RegisterCallback --> scanner_id, status */
base::Callback<void(uint8_t, uint8_t)> registerCallback_vvnx = base::Bind([](uint8_t a, uint8_t b) { printf("register_cb scanner_id=%i status=%i\n",a, b);});

/* EnableCallback --> action, status */
base::Callback<void(uint8_t, uint8_t)> enableCallback_vvnx = base::Bind([](uint8_t a, uint8_t b) { printf("enable_cb action=%i status=%i\n",a, b);});

/* FilterParamSetupCallback --> avbl_space, action_type, status */
base::Callback<void(uint8_t, uint8_t, uint8_t)> filterParamSetupCallback_vvnx = base::Bind([](uint8_t a, uint8_t b, uint8_t c)
{ 	printf("filterParamSetupCallback %i %i %i\n",a, b, c);	});


/* FilterConfigCallback --> filt_type, avbl_space, action, status */                          
base::Callback<void(uint8_t, uint8_t, uint8_t, uint8_t)> filterConfigCallback_vvnx = base::Bind([](uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{ 	printf("filterConfigCallback %i %i %i %i\n",a, b, c, d);	});





int main(){
	
	const hw_module_t* module;
	
	// The HAL handle obtained from the shared library. We hold a weak reference to this since the actual data resides in the shared Bluetooth library.
	//hardware/libhardware/include/hardware/bluetooth.h
	const bt_interface_t* hal_iface_;
	
	// The HAL handle that represents the underlying Bluetooth adapter. (idem weak ref... que ci dessus)
	const bluetooth_device_t* hal_adapter_;
	
	base::MessageLoop main_loop;
	
	LOG(INFO) << "Début de main (LOG)";
	printf("Début de main (printf) \n");
	
	//loade libbluetooth.default.so
    int status = hal_util_load_bt_library(&module);
    if (status) {
      printf("Failed to load Bluetooth library \n");
      return 1;
    }
    
    // Open the Bluetooth adapter.
    hw_device_t* device;
    status = module->methods->open(module, BT_HARDWARE_MODULE_ID, &device);
    if (status) {
      printf("Failed to open the Bluetooth module \n");
      return 1;
    }
    
    hal_adapter_ = reinterpret_cast<bluetooth_device_t*>(device);
    hal_iface_ = hal_adapter_->get_bluetooth_interface();
    
    // Initialize the Bluetooth interface. Set up the adapter (Bluetooth DM) API callbacks.
    status = hal_iface_->init(&bt_callbacks);
    if (status != BT_STATUS_SUCCESS) {
      printf("Failed to initialize Bluetooth stack \n");
      return 1;
    }    
    
    status = hal_iface_->enable(true); //avec false s'arrête rapidos...
    if (status != BT_STATUS_SUCCESS) {
      printf("Failed to enable \n");
      return 1;

    }
    
    //btgatt_interface_t défini dans hardware/bt_gatt.h, et créé au runtime par libbtif dans system/bt/btif/src/btif_gatt.cc (donc au final dans la librairie bluetooth.default.do)
    const btgatt_interface_t* gatt_iface = reinterpret_cast<const btgatt_interface_t*>(hal_iface_->get_profile_interface(BT_PROFILE_GATT_ID));  
      
    status = gatt_iface->init(&bt_gatt_callbacks);
    if (status != BT_STATUS_SUCCESS) {
      printf("Failed to initialize gatt \n");
      return 1;
    } 
    
	sleep(1); //obligatoire (faut laisser le temps au hardware de s'allumerr?)
    
    /** scan des devices standard. 
    status = hal_iface_->start_discovery();
    if (status != BT_STATUS_SUCCESS) {
      LOG(INFO) << "Failed to start_discovery";
      return 1;
    }**/
    
    //définition BleScannerInterface dans hardware/ble_scanner.h et le code pour agir et s'incruster dans les fonctions: system/bt/btif/src/btif_ble_scanner.cc
    BleScannerInterface* ble_iface = reinterpret_cast<BleScannerInterface*>(gatt_iface->scanner);
    
    
    ble_iface->RegisterScanner(registerCallback_vvnx);    
    
    /****####Filtres####
     * pas d'autre solution que de modifier la librairie bluetooth.default.so, parce que google a décidé de modifier les scan params avant chaque lancement de scan.
     * tout se passe dans system/bt/stack/btm/btm_ble_gap.cc à BTM_BleObserve 
     * chemin pour arriver à BTM_BleObserve à partir de ble_iface->Scan(true):
     * ble_iface->Scan(true); -> BTA_DmBleObserve (bta/dm/bta_dm_api.cc) crée une struct de type tBTA_DM_API_BLE_OBSERVE et l'envoie avec bta_sys_sendmsg()
		tBTA_DM_MSG une struct pleine de typedefs -> tBTA_DM_API_BLE_OBSERVE ble_observe; -> bta/dm/bta_dm_act.cc ??? mais par quel mécanisme on y passe?? --> BTM_BleObserve dans btm_ble_gap.cc
     * 
     * Pour avoir un scan avec du whitelisting, il faut que le dernier argument passé à btm_send_hci_set_scan_params() soit à 0x01 (bluetooth core specs p 1262) i.e. SP_ADV_WL chez android, et non pas BTM_BLE_DEFAULT_SFP
     * avant le lancement du scan il faut mettre les bdaddr que tu veux whitelister dans la wl du controller
bool wl_status;
RawAddress esp32_1;
RawAddress::FromString("30:ae:a4:47:56:52", esp32_1);
RawAddress esp32_2;
RawAddress::FromString("30:ae:a4:47:57:aa", esp32_2);
wl_status = btm_update_dev_to_white_list(true, esp32_1);
wl_status = btm_update_dev_to_white_list(true, esp32_2);
BTM_TRACE_EVENT("%s status = %i", __func__, wl_status);
wl_status = btm_execute_wl_dev_operation();
BTIF_TRACE_EVENT("%s status 2=%i", __func__, wl_status);
     * 
     * ##rebuilder bluetooth.default.so
     * make bluetooth.default
     * adb push out/target/product/mido/symbols/system/lib64/hw/bluetooth.default.so /system/lib64/hw/
     */ 




    
    
    ble_iface->Scan(true);

    
    
    
	
	main_loop.Run();
	
	return 0;
}
/**non utilisé finalement, mais pourra être utile quand tu voudras passer des valeurs d'ici vers la librairie, pour advertiser par exemple
auto p = std::make_unique<btgatt_filt_param_setup_t>(btgatt_filt_param_setup); https://shaharmike.com/cpp/unique-ptr/	
ble_iface->ScanFilterParamSetup(4, 1, 0, p, filterParamSetupCallback_vvnx);	
std::vector<unsigned char> vector_vide;	
ble_iface->ScanFilterAddRemove(1, 0, 1, 0, 0, NULL, NULL, &esp32_1, 0, vector_vide, vector_vide, filterConfigCallback_vvnx);  
**/
