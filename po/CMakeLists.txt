set(POT_FILE
    kcm_fcitx.pot
)

file(GLOB PO_FILES
    *.po
)

# Update .po files and compile them to binary .gmo files
gettext_create_translations(${POT_FILE} ALL ${PO_FILES})