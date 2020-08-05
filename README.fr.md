# diyscip
DIY Supervision and Control Intxx PxxxSPA with an ESP8266

:uk: [In English](https://github.com/YorffoeG/diyscip/blob/master/README.md)

---

Ce projet apporte des capacités de supervision et de contrôle des PxxxSPA Intxx sans modifications de l'installation initiale. Ainsi ce controlleur se place entre le bloc de commande et le bloc moteur via leur connecteur spécifique à 5 broches.

Il se connecte à votre réseau WiFi et communique avec un serveur MQTT pour le contrôle (MQTT publish) et la commande (MQTT subscribe).


> :warning: **attention:** Ce projet n'est pas affilié à Intxx. Il est distribué dans l'espoir qui peut vous être utile mais SANS AUCUNE GARANTIE. Tout dommage sur votre spa ou la perte de la garantie initiale du produit est de votre responsabilité, y compris toutes les conséquences liées à l'utilisation de ce projet.


![image](https://github.com/YorffoeG/diyscip/blob/master/docs/controller_PCB_V2.jpg)

### Comment le construire ?
Vous avez d'abord besoin du fabriquer la carte ! Les composants sont faciles à trouver, mais ça nécessite quand même d'être à l'aise avec l'électronique.

Comme IDE, j'utilise [Visual Studio Code](https://code.visualstudio.com/) avec [PlatformIO](https://platformio.org/): Gratuit et pour moi, il offre une meilleure ergonomie pour la gestion des sources.

Pour les connections au SPA qui sont spécifiques à Intxx, vous devrez les imprimer en 3D. J'utilise [ceux-ci](https://www.thingiverse.com/thing:4130911) fournis par Psykokwak. Merci à lui de les avoir partagé ! :+1:


#### PCB_V1
Initialement, la carte V1 était construite à base de NodeMcu V3, basé sur l'esp8266. J'ai utilisé [celui-ci](https://www.amazon.fr/dp/B06Y1ZPNMS) pour le prototypage. Utiliser une carte de développement NodeMcu plutôt qu'un simple composant ESP8266 est une question de commodité: elle intégre un convertisseur 5V (du spa) vers 3.3v et surtout elle offre une connection USB pour le chargement et le debug du logiciel. Le schéma électronique est [ici](https://github.com/YorffoeG/diyscip/blob/master/docs/schematic_PCB_V1.jpg) !

**Cette version ne supporte que les modèles de spa SSP-xxx***

#### PCB_V2
Suite au retour d'expérience de la V1, la carte V2 améliore la compatibilité électrique et ajoute le support des modèles SJB de spa. Elle utilise directement un esp8266 et ajoute des convertisseurs de niveau en les signaux TTL (spa) and CMOS (esp8266). J'ai lu de grand débat sur Internet à propos de la tolérance au 5V des IO de l'esp8266, même si ce semble être le cas, ce n'est pas officiellement supporté, donc je préfére suivre les régles de l'art. Un convertisseur de tension de 5V à 3.3V assure également l'alimentation électrique de l'esp.
L'ajout d'un deuxième multipleur analogique permet de rendre la carte plus générique et permet une compatibilité avec dans les modèles de spa Intxx.
Le schéma électronique est [ici](https://github.com/YorffoeG/diyscip/blob/master/docs/schematic_PCB_V2.jpg).

**Cette version supporte maintenant les modèles SSP-xxx et SJB-xxx**

Vous ne vous sentez pas à l'aise avec la fabrication de la carte ? C'est un projet "Do It Yourself" (Fait Le Toi-même) :smile: N'hésitez pas à me contacter par email _diyscip(AT)runrunweb.net_, il se pourrait que j'ai quelques cartes prêtes dans la poche. Mais gardez en tête qu'il est de votre responsabilité de l'utiliser sur votre spa.

### Configurer le controlleur
Et voilà ! le controlleur est en place. Au premier démarrage, vous devez connecter votre téléphone ou ordinateur au réseau WiFi "DIYSCIP_setup", un écran de configuration vous permet de renseigner votre réseau WiFi avec son mot de passe puis les paramétres du serveur MQTT sur lequel vous connecter. Après vérification de vos réglages, le controlleur redémarrer et est prêt à fonctionner.

Pour retourner au mode de configuration, quand votre spa est en marche, appuyez 6 fois rapidement sur le bouton de sélection de l'unité de température, puis éteignez le spa (pas électriquement mais sur le panneau de commande). Le contrôleur redémarre en mode configuration.

![image](https://github.com/YorffoeG/diyscip/blob/master/docs/DIYSCIP_settings.jpg)


### Serveur et application web
J'ai également développé un serveur MQTT et une application Web "Progressive Web App" pour le spa ! Pour le moment elle n'est pas publique mais si vous voulez y connecter votre contrôleur, contactez moi :  _diyscip(AT)runrunweb.net_
Vous pouvez y jetez un coup d'oeil en version demo: https://diyscip.web.app

![image](https://github.com/YorffoeG/diyscip/blob/master/docs/frontend_app.jpg)

### Comprendre le code et la carte
En analysant l'électronique du panneau de commande, 3 fils fournissent une horlode, des données et un signal de bascule à 2 registres à décalages 8 bits (74HC595). La sortie de ces registres allume les leds du panneau (y compris celles des afficheurs 7-digits). Chaque bouton poussoir est également connecté à une sortie de ces registres. Lorqu'un bouton est pressé, la sortie correpondante des registres est connectée au signal data via une resistance de 1kOhm.

Le controlleur DIYSCIP est branché en paralléle sur les signaux entre le bloc commande et le bloc moteur.

Pour la supervision, l'esp8266 embarqué lit le signal data à la fréquence du signal horloge tandis que le signal de bascule découpe le flux de données en trame de 16 bits. En décodant ces trames, vous obtenez l'état des leds et des afficheurs.

Pour le contrôle c'est un peu différent. La carte du DIYSCIP duplique l'électronique des registres à décalage. L'appui sur un bouton est simulé en pilotant un multipleur analogique qui connecte la sortie d'un registre au signal data.

C'est tout, maintenant on peut lire et écrire l'état du spa, simple non ? :smile:

Une thermistor compléte la partie électronique. Comme elle est sur la carte et placée dans le tube du panneau de contrôle, ce n'est pas précis (il peut faire très chaud dans le tube) mais ça fournie une information sur les conditions météo.

Enfin, un client MQTT envoie et reçoit les états et les commandes depuis un serveur MQTT à travers une connection TCP.


### Les MQTT topics
Le controlleur embarque un client MQTT qui utilise le protocole v3.1.1

Publish Topics:
- **spa/model** : _string_ - Le modèle de spa configuré
- **spa/status** :  online | offline (Will topic)
- **spa/sys/version** : _string_ - la version du firmware de la carte
- **spa/sys/wifi** : _number_ - Niveau du signal WiFi de 0 à 100
- **spa/state** : _number_ - value brute des états des leds, utile pour le debug.
- **spa/state/power**  :  on | off
- **spa/state/filter** : on | off
- **spa/state/heater** : on | off
- **spa/state/heatreached** : on | off (on si la température est atteinte)
- **spa/state/bubble** : on | off
- **spa/temp/board** : _number_ - in Celsius degree
- **spa/temp/water** : _number_ - in Celsius degree
- **spa/temp/desired** : _number_ - in Celsius degree
- **spa/sanitizer** : _number_ - (SJB model only) Temps restant de désinfection, 0 si éteint

Subscribe Topics:
- **spa/state/power/set** : on | off
- **spa/state/filter/set** : on | off
- **spa/state/heater/set** : on | off
- **spa/state/temp/desired/set** : _number_ - en degree Celsius de 20 à 40
- **spa/sanitizer/set** : _number_ - (SJB model only) 0 pour éteindre la désinfection, ou la valeur 3, 5, 8 pour mettre en route.
