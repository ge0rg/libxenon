
void call_ctors(void)
{
	typedef void (*pfunc) ();
	extern pfunc __CTOR_LIST__[];
	extern pfunc __CTOR_END__[];
	pfunc *p;
	
	for (p = __CTOR_END__ - 2; p > __CTOR_LIST__; )
	{
		(*p)();
		p--;
	}
}

