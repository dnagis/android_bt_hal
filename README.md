#BT natif android, deuxième passage (mars 2019)

après la lecture de l'organisation bt android dans https://source.android.com/devices/bluetooth

commence par un mimick de ce que fait le service bluetoothtbd, dlopen de la librairie bluetooth.default.so, donc c'est un service.

usage: mettre dans un dossier (par exemple vvnx/) dans system/bt/ -> system/bt/vvnx, ajouter vvnx aux subdirs de system/bt/Android.bp et make bt_vvnx

pour avoir le whitelisting il faut faire des modifications de la librairie bluetooth.default.so car google lance un scan sans whitelist par défaut
et tu ne peux pas le modifier autrement qu'en rebuildant. Les instructions sont dans main.cc

