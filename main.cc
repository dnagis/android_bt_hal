/**
 * piste: system/bt/hal/bluetooth_interface.cc (suivi notre ami bluetoothtbd->main->daemon.cc->bluetooth_interface.cc)
 * 
 * build: mettre dans un dir dans system/bt que tu ajoutes à Android.bp->subdirs
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
    AdapterStateChangedCallback,
    nullptr,
    nullptr,
    device_found_cb, /* device_found_cb */
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    ThreadEventCallback,
    nullptr, /* dut_mode_recv_cb */
    nullptr, /* le_test_mode_cb */
    nullptr  /* energy_info_cb */
};


btgatt_callbacks_t bt_gatt_callbacks = {
	sizeof(btgatt_callbacks_t),
	nullptr,
    nullptr,
    nullptr,
};


/**
void ma_cb(uint8_t scanner_id, uint8_t status) {
	printf("%i %i", scanner_id, status);
}


class BleScannerIfaceVvnx : public BleScannerInterface {
	public:		

};**/




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
    
    sleep(5);
    
    //btgatt_interface_t défini dans hardware/bt_gatt.h
    const btgatt_interface_t* gatt_iface = reinterpret_cast<const btgatt_interface_t*>(hal_iface_->get_profile_interface(BT_PROFILE_GATT_ID));  
      
    status = gatt_iface->init(&bt_gatt_callbacks);
    if (status != BT_STATUS_SUCCESS) {
      printf("Failed to initialize gatt \n");
      return 1;
    } 
    
	/** sleep(5);
    
     avec /etc/bluetooth/bt_stack.conf -> TRC_BTIF=5 me montre des choses
    status = hal_iface_->start_discovery();
    if (status != BT_STATUS_SUCCESS) {
      LOG(INFO) << "Failed to start_discovery";
      return 1;
    }**/
    
    //BleScannerIface* ble_iface = reinterpret_cast<BleScannerIface*>(gatt_iface->scanner);
    //ble_iface->Scan(true);
    //ble_iface->RegisterScanner(BleScannerInterface::RegisterCallback);
    
    //BleScannerIfaceVvnx ble_iface = reinterpret_cast<BleScannerIfaceVvnx>(gatt_iface->scanner);
    //ble_iface::RegisterCallback = reinterpret_cast<Callback>(ma_cb);
    
	
	main_loop.Run();
	
	return 0;
}
