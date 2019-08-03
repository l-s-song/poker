#!/bin/bash

cd art

./export_images.sh PokerTable.svg

mv PokerTable.svg__layer3.png ../images/table.png
mv PokerTable.svg__layer2.png ../images/act_buttons.png
mv PokerTable.svg__layer4.png ../images/player_button.png

