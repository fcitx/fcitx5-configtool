#!/bin/bash

gen_pot kde:appdata:desktop:ui fcitx5-configtool po/fcitx5-configtool layout/ src/configtool/ src/lib/ src/migrator/
gen_pot kde:json:desktop:ui kcm_fcitx5 po/kcm_fcitx5 src/kcm/ src/plasmathemegenerator/

gen_json po/kcm_fcitx5 src/kcm/kcm_fcitx5.json
