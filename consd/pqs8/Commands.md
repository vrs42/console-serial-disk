*** PQS8 Commands ***

The following commands are built into the PQS8 monitor:

Command	Obsolete	Summary
-------	--------	-------
CAtalog	CAtalog		Print the TFS catalog on the TTY
ECho		Turn on/off command echo
EDit		Edit within a line (like FOCAL Modify)
ENter	ENter	Create a TFS directory entry
ERase	SCratch	Remove lines from buffer
FEtch	LOad	Load a file into the buffer for editing
HAlt	HAlt	Halt the machine
	HRead	Begin auto-sequenced input (HSR if available)
KIll	DElete	Delete a TFS file
LAst	LAst	Report the highest line number in use
LCat	SYstem	Load the CAtalog for the specified default unit
LEft	LEft	Report character count left in buffer
LIst	TYpe	List buffer contents
LNumber	NUmber	Begin autosequenced input
LTape	PUnch	List without line numbers
MOnitor	MOnitor	Reboot the monitor
PLease	PAuse	Print a message and wait for reponse
R	R	Run a system program
RAdix	RAdix	Change input radix
REseq	SEq	Renumber lines in the buffer
RUn	RUn	Run a system program (See R)
SAve	WRite	Save the buffer as a TFS file
TAb		Change tabbing mode
^^		Begin comment

Every newish PQS8 monitor should recognize these, and only the first two
letters of the command are checked.  The second column documents the old
name for many of the commands.

These commands relate to the editing and saving of programs.

You can also edit the current program by typing line numbers and text,
in a manner similar to BASIC.

File names in the CAtalog may be 1-6 characters. A trailing ":u", where "u"
is a unit number (0-7), can be used to specify which unit the file is on.

Most contexts (including filenames) consider text to be monocase (uppercase
or punctuation). (The representation is sixbit, to conserve memory.)

All files in the TFS CAtalog are 2048 words long (16 blocks).

The special filenames "%" and "$" are allocated outside the normal TFS
file area.  They are placed early on the drive, which can dramatically
speed up seek times when the system device is tape.



A number of additional "commands" are often implemented:

Command	Description
------- -----------
ALLCAT	Cache CAtalogs for all units, not just unit 0 (8K)
BATCH	Begin batch commands
BIN	Load a binary program or paper tape
BLKCPY	Block oriented copy/compare
BLKODT	Like ODT, but for media, not memory
BSAVE	Save memory to TFS files
CHANGE	Search and replace in the edit buffer
CONSOL	Enable or Disable the console overlay
CONVRT	
CORE	Set or report memory available
DATE	Set or report the system date
DIRECT	Print TFS or system directory
DT4MAT	DECtape formatter
DTCOPY	DECtape copier
DUMP	Dump or manipulate raw blocks
FILMAN	Manage TFS files
FIND	Search the edit buffer
FOCAL	Start the FOCAL interpreter
GET	Get a binary program ready to run
L6DCON	Read/write LAP6-DIAL/DIAL-MS files
MAP	Create a load map
MARK12	LINCtape/DECtape formatter (PDP-12)
ODT	Start the debugger
OS8CON	Convert files to/from OS/8
PAL	Assemble a program
PRINT	Print files on the line printer
RK4MAT	RK formatter
SET	Set configuration options
START	Start the program
SYSTAT	Print information about the system
SYSTYP	See SYSTAT
TC12F	DECtape manipulation (PDP-12)
TD4MAT	TD8E DECtape formatter
TDCOPY	TD8E DECtape copier
VT	

These are really programs you can run (if they are present). Most of
these you have to know how to use, and spell the program name in it's
entirety.

The most common are BATCH, BIN, CHANGE, DUMP, FIND, GET, START, and ODT.
(These are written by default to new bootable media.)

Note that only BLKCPY actually knows how to use non-system handlers!

Examples of commands of this type which you might commonly use include:

.DATE 4/1/2026
.DIRECT/T/N
.DIRECT/T/S/N
.CORE
.SYSTAT/T

The first DIRECT command is similar to the built-in CAtalog, while the
second lists the system catalog, which enumerates the system programs
(the second category above).  Often the /T (terminal) option is helpful,
as many programs default to output on the line printer. The /N option
suppresses pagination, which may also be helpful.



Here is a sequence to assemble and run a simple program:

.FETCH HELLOW
.LIST
450	$
.WRITE FOO
.PAL B<FOO
.GET B
.START
BUGBUG: TODO: Fix this to be a working example!



For more information, consult the "PQS8 Keyboard Monitor Command Guide.pdf"
or "PQS8 System Programs Gude.pdf".
