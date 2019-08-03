#!/bin/bash

if [ -z $1 ]; then
  echo "Please pass in a filename for the first argument"
  exit
fi

# first set a list of the layers, this is extracted from the list of all id's using inkscapes
# -s switch and then grep-ing for only the matched part (-o) of an extended regexp (-E)
# that matches "layer" followed by between 1 and 3 numbers.

for layer in $(inkscape -S $1 | grep -o -E "^(layer[0-9]{1,3})")
do
  # this line exports the id's only (without that you get overlapping bits from other layers
  # and uses the id's of the layers in the $layer list from above
  # you can add extra requirements when you run the script, they get passed through to $2,
  # $1 is the filename
  inkscape --without-gui --export-id=$layer --export-id-only --export-png=$1__$layer.png $2 $1 > /dev/null

  # format the output slightly, escaping as "echo *" is a bash command
  echo \*\*\* exported $1, $layer \*\*\*
  echo " "
done

