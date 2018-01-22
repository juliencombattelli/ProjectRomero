Corentin Egreteau
19/01/2018

# Adaptation of Lucien's code to Romero Project
(original code: https://github.com/daihitsuji/Project_Car) 

1) La partie STM du projet Romero a utilisé comme base le projet réalisé par Lucien.
Nous avons conservé la même structure en ajoutant nos propres librairies
dans le dossier CAN.

2) Pour lancer le projet sur Keil il suffit de lancer le fichier voiture_test.uvprojx dans le dossier MDK-ARM

3) Pour notre projet les fichiers essentiels pour la compréhension du projet sont :
	Code de Romero:
		app/main.c 		
		CAN/src/API_CAN.c
		CAN/inc/API_CAN.h
	Code du driver CAN de Keil adapté pour la Nucléo / code étudié en cours de réseau intergiciel
		CAN/src/CAN.c
	Code de Lucien:
		lib/Drivers_Car/src/us_sensors.c
		lib/Drivers_Car/src/front_motor.c
		lib/Drivers_Car/src/rear_motor.c
		lib/Drivers_Car/src/direction.c
		lib/Drivers_Car/src/speed_sensors.c
	 