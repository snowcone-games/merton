TARGET = windows
ARCH = %%Platform%%
BIN = merton.exe

OBJS = \
	src\csync.obj \
	src\rcore.obj \
	src\loader.obj \
	src\main.obj

RESOURCES = \
	assets\$(TARGET)\icon.res \
	assets\$(TARGET)\versioninfo.res

INCLUDES = \
	-I. \
	-Isrc \
	-I..\libmatoya\src

DEFS = \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN

RFLAGS = \
	-Isrc \
	/nologo

FLAGS = \
	/W4 \
	/MT \
	/MP \
	/volatile:iso \
	/wd4100 \
	/wd4152 \
	/wd4201 \
	/nologo

LINK_FLAGS = \
	/subsystem:windows \
	/nodefaultlib \
	/manifest:embed \
	/manifestinput:assets\$(TARGET)\embed.manifest \
	/nologo

LIBS = \
	..\libmatoya\bin\$(TARGET)\$(ARCH)\matoya.lib \
	libvcruntime.lib \
	libucrt.lib \
	libcmt.lib \
	kernel32.lib \
	windowscodecs.lib \
	user32.lib \
	comdlg32.lib \
	shell32.lib \
	d3d11.lib \
	dxgi.lib \
	dxguid.lib \
	ole32.lib \
	uuid.lib \
	winmm.lib \
	shcore.lib \
	bcrypt.lib \
	userenv.lib \
	shlwapi.lib \
	advapi32.lib \
	ws2_32.lib \
	gdi32.lib \
	imm32.lib \
	winhttp.lib \
	crypt32.lib \
	cabinet.lib

!IFDEF DEBUG
FLAGS = $(FLAGS) /Ob0 /Zi /Oy-
LINK_FLAGS = $(LINK_FLAGS) /debug
DEFS = $(DEFS) -DMTN_DEBUG
!ELSE
FLAGS = $(FLAGS) /O2 /GS- /Gw /GL
LINK_FLAGS = $(LINK_FLAGS) /LTCG
!ENDIF

CFLAGS = $(INCLUDES) $(DEFS) $(FLAGS)

all: clean clear zip $(OBJS) $(RESOURCES)
	link /out:$(BIN) $(LINK_FLAGS) *.obj $(LIBS) $(RESOURCES)

zip:
	compress\$(TARGET)\mcompress ui-zip.h UI_ZIP src\ui

clean:
	@-del /q $(RESOURCES) 2>nul
	@-del /q *.obj 2>nul
	@-del /q *.exe 2>nul
	@-del /q *.ilk 2>nul
	@-del /q *.pdb 2>nul
	@-del /q ui-zip.h 2>nul

clear:
	@cls
