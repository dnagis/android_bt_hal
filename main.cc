/**
 * piste: system/bt/hal/bluetooth_interface.cc (suivi notre ami bluetoothtbd->main->daemon.cc->bluetooth_interface.cc)
 * 
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

void AdapterStateChangedCallback(bt_state_t state) {
  printf("VVNX Adapter state changed");
}

void ThreadEventCallback(bt_cb_thread_evt evt) {
}


// The HAL Bluetooth DM callbacks.
bt_callbacks_t bt_callbacks = {
    sizeof(bt_callbacks_t),
    AdapterStateChangedCallback,
    nullptr,
    nullptr,
    nullptr, /* device_found_cb */
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


int main(){
	
	const hw_module_t* module;
	
	// The HAL handle obtained from the shared library. We hold a weak reference to this since the actual data resides in the shared Bluetooth library.
	//hardware/libhardware/include/hardware/bluetooth.h
	const bt_interface_t* hal_iface_;
	
	// The HAL handle that represents the underlying Bluetooth adapter. (idem weak ref... que ci dessus)
	const bluetooth_device_t* hal_adapter_;
	
	
	printf("Début de main\n");
	
	//loade libbluetooth.default.so
    int status = hal_util_load_bt_library(&module);
    if (status) {
      printf("Failed to load Bluetooth library: ");
      return 1;
    }
    
    // Open the Bluetooth adapter.
    hw_device_t* device;
    status = module->methods->open(module, BT_HARDWARE_MODULE_ID, &device);
    if (status) {
      printf("Failed to open the Bluetooth module");
      return 1;
    }
    
    hal_adapter_ = reinterpret_cast<bluetooth_device_t*>(device);
    hal_iface_ = hal_adapter_->get_bluetooth_interface();
    
    // Initialize the Bluetooth interface. Set up the adapter (Bluetooth DM) API
    // callbacks.
    status = hal_iface_->init(&bt_callbacks);
    if (status != BT_STATUS_SUCCESS) {
      printf("Failed to initialize Bluetooth stack");
      return 1;
    }
    
    sleep(10);
    
    status = hal_iface_->enable(false);
        if (status != BT_STATUS_SUCCESS) {
      printf("Failed to enable");

    }
	
	sleep(60);
	
	return 0;
}