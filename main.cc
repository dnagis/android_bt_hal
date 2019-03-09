/**
 * piste: system/bt/hal/bluetooth_interface.cc (suivi notre ami bluetoothtbd->main->daemon.cc->bluetooth_interface.cc)
 * 
 * build: mettre dans un dir dans system/bt que tu ajoutes à Android.bp->subdirs
 * 
 * 
 * /etc/bluetooth/bt_stack.conf gère le log, par élément/librairie (btif, stack, ... u name it)
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
		printf("VVNX scan result cb bdaddr=%s \n", addrstr.c_str());		
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



//un grand merci à https://www.chromium.org/chromium-os/packages/libchromeos
base::Callback<void(uint8_t, uint8_t)> registerCallback_vvnx = base::Bind([](uint8_t a, uint8_t b) { printf("register_cb scanner_id=%i status=%i\n",a, b);});

/* filt_type, avbl_space, action, status */                          
base::Callback<void(uint8_t, uint8_t, uint8_t, uint8_t)> filterConfigCallback_vvnx = base::Bind([](uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{ 
	printf("filterConfigCallback_vvnx %i %i %i %i\n",a, b, c, d);	
	});


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
    
    //sleep(5);
    
    //btgatt_interface_t défini dans hardware/bt_gatt.h
    const btgatt_interface_t* gatt_iface = reinterpret_cast<const btgatt_interface_t*>(hal_iface_->get_profile_interface(BT_PROFILE_GATT_ID));  
      
    status = gatt_iface->init(&bt_gatt_callbacks);
    if (status != BT_STATUS_SUCCESS) {
      printf("Failed to initialize gatt \n");
      return 1;
    } 
    
	sleep(1); //obligatoire (faut laisser le temps au hardware de s'allumerr?)
    
    /** scan des devices standard. avec /etc/bluetooth/bt_stack.conf -> TRC_BTIF=5 j'ai du logcat touffu
    status = hal_iface_->start_discovery();
    if (status != BT_STATUS_SUCCESS) {
      LOG(INFO) << "Failed to start_discovery";
      return 1;
    }**/
    
    BleScannerInterface* ble_iface = reinterpret_cast<BleScannerInterface*>(gatt_iface->scanner);
    
    //system/bt/types/
    RawAddress esp32_1;
	RawAddress::FromString("30:ae:a4:47:56:52", esp32_1);
    
    ble_iface->ScanFilterAddRemove(1, 0, 0, NULL, NULL, NULL, NULL, &esp32_1, NULL, std::vector<unsigned char>(), std::vector<unsigned char>(), filterConfigCallback_vvnx);
    
    ble_iface->RegisterScanner(registerCallback_vvnx);
    
    
    
    
    ble_iface->Scan(true);
    
    

    
    
    
	
	main_loop.Run();
	
	return 0;
}
