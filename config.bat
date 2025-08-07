@rem ----- Builtch Configuration -----
@rem --------- Version 0.2.2 ---------

@rem ------------- Files -------------
set source_files=autostereogram2.c
set output_file=autostereogram2.exe

@rem ----------- Arguments -----------
set common_args=-Wall -Werror=return-type -Werror=int-conversion -Werror=implicit-function-declaration
set debug_args=-D _DEBUG
set release_args=-D NDEBUG -O3
set test_args=-D _DEBUG -D TESTING

@rem ----------- Libraries -----------
call libgan require raylib 5.0 || exit /b 1

@rem I don't know why, but you have to add this.
@rem Otherwise this doesn't always return 0 when used in cmd.
@rem I think it's fine in other terminals.
exit /b 0 
