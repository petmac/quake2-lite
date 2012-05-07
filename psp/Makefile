TARGET = Quake2

OBJS = \
	gu_display_list.o \
	gu_draw.o \
	gu_image.o \
	gu_light.o \
	gu_mesh.o \
	gu_model.o \
	gu_rmain.o \
	gu_rmisc.o \
	gu_rsurf.o \
	gu_vram.o \
	gu_warp.o \
	\
	net_psp.o \
	sys_psp.o \
	vid_menu_psp.o \
	vid_psp.o \
	\
	../client/cl_cin.o \
	../client/cl_ents.o \
	../client/cl_fx.o \
	../client/cl_input.o \
	../client/cl_inv.o \
	../client/cl_main.o \
	../client/cl_newfx.o \
	../client/cl_parse.o \
	../client/cl_pred.o \
	../client/cl_scrn.o \
	../client/cl_tent.o \
	../client/cl_view.o \
	../client/console.o \
	../client/keys.o \
	../client/menu.o \
	../client/qmenu.o \
	../client/snd_dma.o \
	../client/snd_mem.o \
	../client/snd_mix.o \
	\
	../game/g_ai.o \
	../game/g_chase.o \
	../game/g_cmds.o \
	../game/g_combat.o \
	../game/g_func.o \
	../game/g_items.o \
	../game/g_main.o \
	../game/g_misc.o \
	../game/g_monster.o \
	../game/g_phys.o \
	../game/g_save.o \
	../game/g_spawn.o \
	../game/g_svcmds.o \
	../game/g_target.o \
	../game/g_trigger.o \
	../game/g_turret.o \
	../game/g_utils.o \
	../game/g_weapon.o \
	../game/m_actor.o \
	../game/m_berserk.o \
	../game/m_boss2.o \
	../game/m_boss3.o \
	../game/m_boss31.o \
	../game/m_boss32.o \
	../game/m_brain.o \
	../game/m_chick.o \
	../game/m_flash.o \
	../game/m_flipper.o \
	../game/m_float.o \
	../game/m_flyer.o \
	../game/m_gladiator.o \
	../game/m_gunner.o \
	../game/m_hover.o \
	../game/m_infantry.o \
	../game/m_insane.o \
	../game/m_medic.o \
	../game/m_move.o \
	../game/m_mutant.o \
	../game/m_parasite.o \
	../game/m_soldier.o \
	../game/m_supertank.o \
	../game/m_tank.o \
	../game/p_client.o \
	../game/p_hud.o \
	../game/p_trail.o \
	../game/p_view.o \
	../game/p_weapon.o \
	../game/q_shared.o \
	\
	../null/cd_null.o \
	../null/in_null.o \
	../null/snddma_null.o \
	\
	../qcommon/cmd.o \
	../qcommon/cmodel.o \
	../qcommon/common.o \
	../qcommon/crc.o \
	../qcommon/cvar.o \
	../qcommon/files.o \
	../qcommon/md4.o \
	../qcommon/mem.o \
	../qcommon/net_chan.o \
	../qcommon/pmove.o \
	\
	../server/sv_ccmds.o \
	../server/sv_ents.o \
	../server/sv_game.o \
	../server/sv_init.o \
	../server/sv_main.o \
	../server/sv_send.o \
	../server/sv_user.o \
	../server/sv_world.o

INCDIR = 
CFLAGS = -O2 -G0 -DGAME_HARD_LINKED -DREF_HARD_LINKED
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

;LIBDIR = $(PSPSDK)/lib
LIBS = -lm -lpsprtc -lpspgum -lpspgu
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Quake 2 ported by Peter Mackay

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

deploy: all
	psp-readelf --sections --wide $(TARGET).elf >sections.txt
	psp-nm --size-sort --reverse-sort --radix=d $(TARGET).elf >symbols.txt
	$(MKDIR) -p G:/PSP/GAME/Quake2
	$(CP) EBOOT.PBP G:/PSP/GAME/Quake2/EBOOT.PBP
	$(CP) PARAM.SFO G:/PSP/GAME/Quake2/PARAM.SFO
