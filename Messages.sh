#!/bin/bash

gen_pot kde:appdata:desktop:ui fcitx5-configtool po/fcitx5-configtool layout/ src/configtool/ src/lib/ src/migrator/
gen_pot kde:appdata:desktop:ui kcm_fcitx5 po/kcm_fcitx5 src/kcm/ src/plasmathemegenerator/

gen_json kcm_fcitx5 po/kcm_fcitx5 src/kcm_fcitx5.json
