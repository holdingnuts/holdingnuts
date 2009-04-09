#!/bin/bash
#
#  Copyright 2008, 2009, Dominik Geyer
#
#  This file is part of HoldingNuts.
#
#  HoldingNuts is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  HoldingNuts is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with HoldingNuts.  If not, see <http://www.gnu.org/licenses/>.
#
# Usage: compose_cards.sh < 2 | 4 >


density=80
font=Arial  #Eras-Normal  #Baskerville-Normal
font_ratio=35
font_kerning=-4

ratio_suit_small=34
ratio_suit_big=50

color_s='rgb(0,0,0)'
color_h='rgb(212,0,0)'
color_d='rgb(0,0,255)'
color_c='rgb(0,128,0)'

################################################################################

faces="A K Q J 10 9 8 7 6 5 4 3 2"
suits="s h d c"

img_width()
{
	identify -format '%w' $1
}

img_height()
{
	identify -format '%h' $1
}

create_template()
{
	convert -density ${density} -background none card.svg tmp/card.png
}

create_face()
{
	ps=$((tw * font_ratio / 100))

	case ${2} in
		s) color=${color_s} ;;
		h) color=${color_h} ;;
		d) color=${color_d} ;;
		c) color=${color_c} ;;
	esac

	convert -font ${font} -pointsize ${ps} -background none \
		-fill ${color} -kerning ${font_kerning} label:"${1}"  \
		tmp/face_${1}.png
	
	convert tmp/face_${1}.png -rotate 180 tmp/face_${1}_flip.png
}

create_suit()
{
	ratio_small=$((density * ratio_suit_small / 100))
	ratio_big=$((density * ratio_suit_big / 100))
	
	if [[ ${four_color} == 1 ]] ; then
		if [[ ${1} == d || ${1} == c ]] ; then
			prefix="_4"
		fi
	fi
	
	convert -density ${ratio_small} -background none suit_${1}${prefix}.svg tmp/suit_${1}.png
	#convert -flip tmp/suit_${1}.png tmp/suit_${1}_flip.png

	convert -density ${ratio_big} -background none suit_${1}${prefix}.svg tmp/suit_${1}_big.png
}

create_card()
{
	if [[ $1 == "10" ]] ; then
		name=T
	else
		name=$1
	fi

	# big suit middle
	sb_w=$(img_width tmp/suit_${2}_big.png)
	sb_h=$(img_height tmp/suit_${2}_big.png)
	sb_x=$((tw / 2 - sb_w / 2))
	sb_y=$((th / 2 - sb_h / 2))

	# small suit top right
	s1s_w=$(img_width tmp/suit_${2}.png)
	s1s_h=$(img_height tmp/suit_${2}.png)
	s1s_x=$((tw - s1s_w - s1s_w / 2))
	s1s_y=$((s1s_h - s1s_h / 2))
	
	# small suit bottom left
	s2s_x=$((s1s_w - s1s_w / 2))
	s2s_y=$((th - s1s_h - s1s_h / 2))

	# face top left
	f1_w=$(img_width tmp/face_${1}.png)
	f1_h=$(img_height tmp/face_${1}.png)
	f1_x=$((f1_h / 4))
	f1_y=$((f1_h / 8))
	
	# flipped face bottom right
	f2_x=$((tw - f1_w - f1_x))
	f2_y=$((th - f1_h - f1_y))

	convert tmp/card.png \
		tmp/suit_${2}_big.png -geometry +${sb_x}+${sb_y} -composite \
		tmp/suit_${2}.png -geometry +${s1s_x}+${s1s_y} -composite  \
		tmp/suit_${2}.png -geometry +${s2s_x}+${s2s_y} -composite  \
		tmp/face_${1}.png -geometry +${f1_x}+${f1_y} -composite \
		tmp/face_${1}_flip.png -geometry +${f2_x}+${f2_y} -composite \
		output/${name}${2}.png
}

create_back_card()
{
	ratio=$((density * 35 / 100))
	
	convert -density ${ratio} -background none emblem.svg tmp/emblem.png
	
	e_w=$(img_width tmp/emblem.png)
	e_h=$(img_height tmp/emblem.png)
	
	e_x=$((tw / 2 - e_w / 2))
	e_y=$((th / 2 - e_h / 2))
	
	convert tmp/card.png \
		tmp/emblem.png -geometry +${e_x}+${e_y} -composite \
		output/back.png
}

create_blank_card()
{
	convert -size ${tw}x${th} -background none NULL: output/blank.png
}

################################################################################

if [[ $# != 1 ]]; then
	echo "Usage: compose_cards.sh < 2 | 4 >" >&2
	exit
fi


decktype=$1

if [[ ${decktype} == "4" ]] ; then
	echo "Creating 4-color deck"
	four_color=1
elif [[ ${decktype} == "2" ]] ; then
	echo "Creating 2-color deck"
	
	# use black/red for diamonds and clubs
	color_d="${color_h}"
	color_c="${color_s}"
else
	echo "Unknown deck type." >&2
	exit 1
fi


mkdir -p tmp output

# create the card surface
create_template


# get card surface width and height
tw=$(img_width tmp/card.png)
th=$(img_height tmp/card.png)

echo "Output size: ${tw}x${th}"

for s in ${suits} ; do
	echo "Creating suit ${s}"
	create_suit ${s}
	
	echo -n "Creating cards "
	for f in ${faces} ; do
		echo -n "${f}${s} "
		create_face ${f} ${s}
		create_card ${f} ${s}
	done
	
	echo
done

echo "Creating back card"
create_back_card

echo "Creating blank card"
create_blank_card
