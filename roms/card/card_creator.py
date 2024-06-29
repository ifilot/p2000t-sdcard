# -*- coding: utf-8 -*-

import os
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(__file__)

# load templates
switch_down = Image.open(os.path.join(ROOT, 'switch_down.png'))
switch_down.thumbnail((int(111/2), int(287/2)))
switch_up = Image.open(os.path.join(ROOT, 'switch_up.png'))
switch_up.thumbnail((int(111/2), int(287/2)))

# create new image
card = Image.new(mode="RGBA", size=(3500, 2000), color=(255,255,255,00))

# specify tags
GAME = 1
EXROM = 2
ROML = 4
ROMH = 8

# specify game data
data = [
    'BASIC NL v1.1',
    'BASIC NL v1.1a2',
    'BASIC EN v1.0',
    'JWS Basic',
    'SD-CARD Launcher',
    'SD-CARD Flasher',
    'RAM expansion test',
    'Maintenance Utility',
    'Forth',
    'Assembler 5.9',
    'Zemon v1.4',
    'Flexbase v1.6',
    'Familiegeheugen v4',
    'Tekstverwerker v1',
    'Text 2000 v3',
    'Texteditor DE v2',
    'Word Processor v2',
]

# specify font
fnt = ImageFont.truetype(os.path.join(ROOT, 'OpenSans-ExtraBold.ttf'), 78)
d = ImageDraw.Draw(card)

# specify break point
breakpt = 20

pos = 0
ypos = 100
xpos = 100
cnt = 0
for i,label in enumerate(data):
    switch = format(i, '#07b')
    print(switch)
    d.text((xpos,ypos+15), label, font=fnt, fill=(0,0,0))
        
    # draw dipswitch box    
    xpos += 1000   
    d.rectangle((xpos-20, ypos-20, xpos+380, ypos+175), fill=(240,30,30), outline=(0,0,0), width=5)
    
    for s in switch[2:]:
        if s == "1":
            card.paste(switch_up, (xpos, ypos))
            xpos += 75
        else:
            card.paste(switch_down, (xpos, ypos))
            xpos += 75
    
    if i == 8:
        ypos = 100
    else:
        ypos += 200
        
    if i >= 8:
        xpos = 1800
    else:
        xpos = 100

#card.show()
card.save('card.png')