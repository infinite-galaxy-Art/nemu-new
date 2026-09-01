SECTIONS
{
	. = 0x800000;
	.text :
	{
		obj/testcase/start.o(.text)
		*(.text)
	}
}
