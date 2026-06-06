# Trigger ID Search
find stuff by ids !

## How To Use
oki so this mod modifies the find object by group popup (its the magnifying glass in the delete menu)

in the bottom right are the main, bottom to top the buttons are
- find by group id
- find by item id
- find by collision (block) id
in the top right is a shortcut to the mods settings
in the bottom left is a toggler that toggles *find group mode*

if u want the normal find object by group functionality the popup still works like normal if u click the ok button

when using the custom buttons tho u can input a list (separated by commas) of groups (by default this acts as an `or` operation but u can change that in settings)

filtering by how the id is used is also supported

by default if u have any selected objs that will act as a filter and the mod will only find objs in ur selection

- ***Note*** collectibles arent supported rn cuz i cant be fucked to go thru every single collectible with a target group and add it to my api for triggers i already spent too many hours on that shit [go pr it or smth if u want it](https://github.com/Nwo5-trg/Nwo5-Silly-API/blob/main/include/editor/trigger.hpp)

- ***Note*** in order to find counter labels and collision blocks *find group mode* has to be enabled (cuz i arbitrarily chose to classify that as an id the obj has instead of an id its targeting)

## Credits
- ***CarlIsBored*** (on the gd modding discord server) for the mod idea
- ***Rue*** for all the fire assets and logo