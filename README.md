# DictoSinico

A GUI for the free and Creative Commons Chinese-English dictionary. 

Clicking on a cell in the table displays the contents of the row. 

The "Animate" button plays back a GIF animation of either the traditional or the simplified forms of the characters in a given word, while clicking on the "Stop" button will show the same characters without animation. 

The user can enable three additional options in preferences: showing pinyin with diacritics instead of tone numbers; coloring pinyin syllables to aid memorization (1: red, 2: orange, 3: green, 4: blue, 5: black); showing the highest HSK level of any of the characters in the word. 

## License

The original code is under the Gnu General Public License v. 3.0. (https://www.gnu.org/licenses/gpl-3.0.html) The CC-CEDICT (https://cc-cedict.org/wiki/) text file is under the Creative Commons Attribution-Share Alike 3.0 License (http://creativecommons.org/licenses/by-sa/3.0/). The CSV file with the HSK level for individual characters comes from this Kaggle dataset: https://www.kaggle.com/datasets/ruddfawcett/hanzidb and is under the Open Data Commons Open Database License (ODbL) v1.0. (https://opendatacommons.org/licenses/odbl/1-0/) All three fonts used in the software are under the SIL Open Font License (https://scripts.sil.org/OFL). The makemeahanzi characters (ths SVG files and the GIF files generated from them) are under the Arphic Public License (https://ftp.gnu.org/non-gnu/chinese-fonts-truetype/LICENSE).

The makemeahanzi characters in the "svgs/" folder simply display the characters in black, while those in the "gifs/" folder contain the same animation as those in the original "svgs" folder of the makemehanzi project. (https://github.com/skishore/makemeahanzi)

## Building

The project uses Qt version 17 with QMake. Using QtCreator as IDE is recommended. 