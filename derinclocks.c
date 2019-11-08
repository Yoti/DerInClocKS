#include <taihen.h>
#include <string.h>
#include <stdio.h>
#include "blit.h"
#include <psp2kern/power.h> 

static SceUID g_hooks[1];

int (*_kscePowerGetGpuEs4ClockFrequency)(int*, int*);
int (*_kscePowerGetGpuClockFrequency)(void);
int (*_ksceKernelGetModuleInfo)(SceUID, SceUID, SceKernelModuleInfo *);
int (*_ksceKernelGetModuleList)(SceUID pid, int flags1, int flags2, SceUID *modids, size_t *num);
int r1, r2;
#define kscePowerGetGpuEs4ClockFrequency _kscePowerGetGpuEs4ClockFrequency
#define kscePowerGetGpuClockFrequency _kscePowerGetGpuClockFrequency
#define ksceKernelGetModuleInfo _ksceKernelGetModuleInfo
#define ksceKernelGetModuleList _ksceKernelGetModuleList

void clocks() {
	kscePowerGetGpuEs4ClockFrequency(&r1, &r2);
	blit_stringf(20, 50, "ARM: %d", kscePowerGetArmClockFrequency());
	blit_stringf(20, 60, "BUS: %d", kscePowerGetBusClockFrequency());
	blit_stringf(20, 70, "GPU: %d", kscePowerGetGpuClockFrequency());
	blit_stringf(20, 80, "XBA: %d", kscePowerGetGpuXbarClockFrequency());
	blit_stringf(20, 90, "ES4: %d", r1);
}

static tai_hook_ref_t ref_hook0;
int _sceDisplaySetFrameBufInternalForDriver(int fb_id1, int fb_id2, const SceDisplayFrameBuf *pParam, int sync){
	if(fb_id1 && pParam) {
		SceDisplayFrameBuf kfb;
		memset(&kfb,0,sizeof(kfb));
		memcpy(&kfb, pParam, sizeof(SceDisplayFrameBuf));
		blit_set_frame_buf(&kfb);
		blit_set_color(0x0000FF00, 0xff000000);
		clocks();
	}
		
	return TAI_CONTINUE(int, ref_hook0, fb_id1, fb_id2, pParam, sync);
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {	
	module_get_export_func(KERNEL_PID, "ScePower", 0x1590166F, 0x475BCC82, &_kscePowerGetGpuEs4ClockFrequency);
	module_get_export_func(KERNEL_PID, "ScePower", 0x1590166F, 0x64641E6A, &_kscePowerGetGpuClockFrequency);

	if(module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0xC445FA63, 0xD269F915 , &_ksceKernelGetModuleInfo))
		module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0x92C9FFC2, 0xDAA90093 , &_ksceKernelGetModuleInfo);
	if(module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0xC445FA63, 0x97CF7B4E , &_ksceKernelGetModuleList))
		module_get_export_func(KERNEL_PID, "SceKernelModulemgr", 0x92C9FFC2, 0xB72C75A4 , &_ksceKernelGetModuleList);

	g_hooks[0] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hook0, "SceDisplay",0x9FED47AC,0x16466675, _sceDisplaySetFrameBufInternalForDriver); 

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	if (g_hooks[0] >= 0) taiHookReleaseForKernel(g_hooks[0], ref_hook0);

	return SCE_KERNEL_STOP_SUCCESS;
}
