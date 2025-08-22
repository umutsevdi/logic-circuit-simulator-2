#!/bin/bash
APPNAME_BIN="LogicCircuitSimulator"
init()
{
xgettext \
    --copyright-holder="Umut Sevdi"\
    --language=C++ \
    --keyword=_ \
    --keyword=N_ \
    --add-comments="TRANSLATORS:Umut Sevdi" \
    --package-name=$APPNAME_BIN \
    --package-version="1.0.0" \
    --output=i18n/${APPNAME_BIN}.pot \
    $(find src/ -name '*.cpp' -o -name '*.h')
#    --from-code=UTF-8 \
}


merge_po()
{
    local locales=("fr_FR" "de_DE" "tr_TR" "en_US" "ru_RU")

    for loc in "${locales[@]}"; do
        loc+=".po"
        local po_path="i18n/${loc}"
        if [[ -f "$po_path" ]]; then
            echo "Merging updates into $po_path"
            msgmerge --quiet --update \
                     --no-fuzzy-matching \
                     "$po_path" i18n/${APPNAME_BIN}.pot
        else
            echo "Creating new $po_path"
            msginit --input=i18n/${APPNAME_BIN}.pot \
                    --no-translator \
                    --locale="${loc}" \
                    --output="$po_path"
        fi
    done
}

make_mo()
{
    rm -rf misc/locale
    for i in i18n/*.po; do
        base=`basename -s .po $i`
        target="misc/locale/$base/LC_MESSAGES/"
        mkdir -p $target
        msgfmt -o ${target}/${APPNAME_BIN}.mo $i
    done

}

init
merge_po
make_mo
