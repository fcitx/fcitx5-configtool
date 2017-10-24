#!/bin/bash
DOMAIN=kcm_fcitx5
POT_FILE=po/$DOMAIN.pot
set -x
source_files=$(find . -name \*.cpp -o -name \*.h)
xgettext --keyword=_ --kde --language=C++ --add-comments --sort-output -o ${POT_FILE} $source_files
desktop_files=$(find . -name \*.conf.in -o -name \*.desktop.in)
xgettext --language=Desktop $desktop_files -j -o ${POT_FILE}
ui_files=$(find . -name \*.ui)
extractrc $ui_files > rc.cpp
xgettext --kde --language=C++ --add-comments --sort-output -j -o ${POT_FILE} rc.cpp
rm -f rc.cpp

echo > po/LINGUAS

for pofile in $(ls po/*.po | sort); do
  pofilebase=$(basename $pofile)
  pofilebase=${pofilebase/.po/}
  msgmerge -U --backup=none $pofile ${POT_FILE}
  echo $pofilebase >> po/LINGUAS
done
