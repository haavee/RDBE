*
* skedf.ctl - sked/drudg program control file
*
* Last modified by NRV 951002 for FS9 
*
* This file is free-field except for section names
* which must begin with $ in column 1 and have no
* blanks.  Either upper or lower case is OK for section names.
* Remember path and file names in UNIX are case sensitive.  
*
$catalogs 
*
* Catalog file names: used only by sked.
*
*
$schedules 
*
* Schedule file path:
* Enter here the absolute path for reading/writing schedules. 
* This path is prepended to schedule files by SKED and DRUDG.
* Default is null, i.e. use your local directory.
*
/usr2/sched/
*
*$drudg
*
* DRUDG file path:
* Enter here the absolute path for writing DRUDG output files,
* e.g. SNAP files. This path is prepended to files written by DRUDG. 
* Default is null, i.e. use your local directory.
*
/usr2/sched/
*
* here separate for snap and proc, avoiding permission problem
* when copying !
$snap
/usr2/sched/
$proc
/usr2/proc/
$scratch 
*
* Scratch directory:
* Enter the absolute path for scratch files. 
* This path is prepended to scratch files by SKED and DRUDG.
* Default is null, i.e. use your local directory.
* All scratch files except the SKED command log are
* deleted upon exiting the programs.
*
/tmp/
*
$print
*
* Printer commands: enter printer type for drudg.
* Recognized names: laser, epson, epson24, file. 
printer      laser
*
* Printer scripts: enter name of script to be executed 
* for different types of output. Recognized key words
* are: portrait, landscape, labels. If no label script
* is specified, drudg defaults to using "lpr". If no
* portrait or landscape scripts are specified, drudg
* defaults to embedding escape sequences for the
* appropriate output and uses "recode latin1:ibmpc"
* piped to lpr to print the output file. 
* Formats: 
* portrait <script name for portrait output>
* landscape <script name for landscape output>
* labels <script name for bar code label output>
label_printer postscript
portrait      lpr -Phplj2 $*
landscape     lpr -Phplj2 $*
labels       lpr -Phplj2
* This example is for a laser printer, 6 lines/inch, 10 char/inch:
*portrait lpr -ofp10 -olpi6 $*
*landscape lpr -ofp10 -o1pi6 -olandscape $*
* These examples are the same as above but for a smaller font:
*portrait lpr -ofp16.66 -olpi6 $*
*landscape lpr -ofp16.66 -o1pi6 -olandscape $*
*portrait <script name for portrait output>
*landscape <script name for landscape output>
*
* Output control:
* Enter the desired orientation and font size for listings. Key words
* are option1, option4, and option5 for the three listing options.
* Follow the key word by a 2-letter code, first letter "p" or "l" for
* portrait or landscape, second letter "s" or "l" for small or large
* font. If none are specified, the defaults are as listed below:
*option1 ls (landscape, small font)
*option4 ps (portrait, small font)
*option5 ps (portrait, small font)
*
* Tape label script:
* Enter a script for printing tape labels. If no script is specified,
* the default is to use "lpr" to print the temporary file.
*labels <script name for label printing>
* Tape label printer:
* Enter the name of the label printer. If no name is specified, drudg
* will not attempt to print tape labels. Recognized names are postscript,
* epson, epson24, laser+barcode_cartridge.
label_printer postscript
*
* Label size:
* Specify label size parameters, only valid for "postscript" type. If
* no size is specified, drudg cannot print labels.
* <ht> height of a single label, in inches
* <wid> width of a single label, in inches
* <rows> number of rows of labels on the page
* <cols> number of columns of labels on the page
* <top> offset of the top edge of the first row of labels from the 
*      top of the page, in inches
* <left> the offset of the left edge of the first column of labels
*      from the left side of the page, in inches
* Format:
* label_size <ht> <wid> <rows> <cols> <top> <left>
label_size  1.61  4.11    7     2    -0.8   0.10   Zweckform 3477
$misc
epoch 1950
-------------------
