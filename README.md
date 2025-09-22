You need to use c++ 20 standard or  heigher to compile it.


this project is made from the official notepad++ plugin template.

The point change is in PluginDefinition.cpp and PluginDefinition.h

its used for code snippet.But you could custom more.

As we know, there were some other plugin like "QuickText", 

I want make a snippet function which i could send params ,
then the snippet content use these params.

How does it work?

Step1:

Open a txt file,
write three line:

cmd
$$
Hello $$ World

let me explain it:
first line is to define a keyword,
2nd line is to define a cursor jump placeholder.(any two ascii charactor is ok)
3rd line is the snippet content.(actually , we should call it as "part" not "line",
because you can define a part with multi line content at there)

then, select from start position of first line  to content part with any end position,
keep it be selected , then trigger the plugin function "AddCmd",

if there is no warning , a command called "cmd" is defined.

okay,now lets go to a clear area, a clear line , 
we type "cmd" in one line , dont worry about indentation.
then lets trigger the function of plugin "snippet",
the text of "cmd" is replaced with snippet content.
you can trigger function "JumpCursor" to jump cursor in snippet content.
clean the placeholder.

if the snippet content has placeholder, it will be added a spectial end mark. (automatically)
its normal mark + '@' , 

eg, if you defined a cursor placeholder is "##",
then the end mark is "##@".


if you want send params ?
lets see another example:

cmd p1 p2
$$
Hello *{p1}
My name is *{p2}

select them all and add cmd,
now we go a clear line , 
we type:
cmd Tom Allen

then we trigger snippet,
the content will be replaced with:
Hello Tom
My name is Allen


lets go to the plugin folder,

there is a special file "ConfigMeta.txt"

its a json map , a map of extension name to sence name.

file extension type => you custom name type (we call "sence") => sence.txt

all your command is stored in target sence .txt , in plugin folder.


if you want to build a new sence for some file extension,

step1:
open "ConfigMeta.txt" file,
define a map item , like "txt" => "normalText",

step2:
create a new file , named with "normalText.txt",
fill with content "{}" ,because we should make sure its a leglle json file.

done,
now you can open a txt file,
and define your snippet cmd , it will be saved to target sence.
 











