# Système de Monitoring de Températures et Pressions dans un Échangeur à Plaques

## Description du Projet

Ce projet est un système de monitoring en temps réel des températures et pressions dans un échangeur à plaques situé dans la Halle Technologique d'AgroParisTech. Il est utilisé dans le cadre de travaux pratiques (TP) pour les étudiants, leur permettant de visualiser et d'analyser les performances thermiques de l'échangeur.

L'interface web intégrée permet de suivre les mesures en temps réel via un ESP32, qui collecte les données des capteurs connectés. Le système est conçu pour être simple à déployer et à utiliser, avec une interface intuitive pour la gestion des configurations et des données.

## Installation sur ESP32

### 1. Configuration du Fichier `.ino`

1. **Téléchargez ou clonez le dépôt GitHub** contenant le fichier `.ino`.
2. **Ouvrez le fichier `.ino`** dans l'IDE Arduino.
3. **Connectez votre ESP32** à votre ordinateur.
4. Dans l'IDE Arduino, sélectionnez la **carte ESP32** appropriée sous `Outils > Type de carte`, ainsi que le **port COM** correspondant.
5. **Compilez** et **téléversez** le code sur l'ESP32 en utilisant l'option `Téléverser`.

### 2. Chargement des Fichiers `index.html` et `conf.json` sur l'ESP32

Pour que l'interface web fonctionne correctement, vous devez télécharger les fichiers `index.html` et `conf.json` dans le système de fichiers de l'ESP32 (LittleFS ou SPIFFS).

#### Étape 1 : Préparer les fichiers
- **Créez un dossier nommé `data`** à la racine de votre projet Arduino.
- **Placez les fichiers `index.html` et `conf.json`** dans ce dossier `data`.

#### Étape 2 : Installer le Plugin "ESP32 Sketch Data Upload"
Si ce n'est pas déjà fait, vous devez installer le plugin "ESP32 Sketch Data Upload" pour l'IDE Arduino. Ce plugin permet de téléverser les fichiers du dossier `data` vers le système de fichiers de l'ESP32.

Pour installer le plugin, suivez les instructions sur le lien suivant :
- **[Installer ESP32 Sketch Data Upload](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)**

#### Étape 3 : Téléverser les fichiers
- Dans l'IDE Arduino, allez dans `Outils > ESP32 Sketch Data Upload`.
- **Cliquez sur `ESP32 Sketch Data Upload`** pour téléverser les fichiers du dossier `data` vers l'ESP32.

## Aperçu de l'Interface Web

L'interface web fournie par ce système est conçue pour être intuitive et facile à utiliser.

### Fonctionnalités de l'Interface

- **Suivi en temps réel** : Affiche les températures et les pressions mesurées en temps réel sous forme de graphiques interactifs.
- **Contrôle du système** : Permet de démarrer/arrêter la journalisation des données et de configurer les paramètres du système, comme l'intervalle de temps entre les mesures.
- **Gestion des fichiers** : Offre une interface pour visualiser, télécharger ou supprimer les fichiers de données stockés dans le système de fichiers de l'ESP32.
- **Zoom et Panoramique** : Les graphiques permettent un zoom et un panoramique interactifs pour une analyse détaillée des données.

## Rôle de l'Interface

L'interface web développée pour ce projet sert à :

- **Monitorer les performances de l'échangeur à plaques** en temps réel, en affichant les températures et pressions aux points critiques.
- **Fournir un outil pédagogique** aux étudiants d'AgroParisTech, leur permettant de visualiser les données en temps réel, de les analyser et de comprendre les dynamiques thermiques dans un échangeur à plaques.
- **Faciliter l'accès aux données** pour des analyses ultérieures ou pour le téléchargement des journaux de données dans le cadre des TP.

## Contexte d'Utilisation

Ce système est installé dans la Halle Technologique d'AgroParisTech et est principalement utilisé lors de travaux pratiques par les étudiants. Il aide à illustrer les concepts de transfert thermique et d'échange de chaleur, en fournissant des données réelles que les étudiants peuvent observer et manipuler pour mieux comprendre ces phénomènes.
