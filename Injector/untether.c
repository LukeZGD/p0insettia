/*
 * injector.c
 * copyright (C) 2021/11/14 sakuRdev
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

//#define N42 (1) // n41/n42 only
#define A6_IBOOT_LOADADDR_BASE  (0x80000000)
#define A6_IBOOT_IMAGE_BASE     (0xBFF00000)

#define IBOOT_DATA_START        (0x40)

static int open_file(const char *file, size_t *sz, void **buf){
    FILE *fd = fopen(file, "r");
    if (!fd) {
        printf("error opening %s\n", file);
        return -1;
    }
    
    fseek(fd, 0, SEEK_END);
    *sz = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    *buf = malloc(*sz);
    if (!*buf) {
        printf("error allocating file buffer\n");
        fclose(fd);
        return -1;
    }
    
    fread(*buf, *sz, 1, fd);
    fclose(fd);
    
    return 0;
}

static void patch_iboot(const char* path)
{

    void* buf;
    size_t size;
    
    char str[] = "rd=md0 serial=3 debug=0x14\x00";
    
    uint32_t rsa_patch;
    uint32_t debug_patch;
    uint32_t bootArgs_normal;
    uint32_t bootArgs_rdsk1;
    uint32_t bootArgs_rdsk2;
    uint32_t bootmode;
    uint32_t goCmd;
    uint32_t bootRamdisk;
    uint32_t bootRamdiskStr;
    uint32_t bootPartition;
    uint32_t bootPartitionStr;
    uint32_t bootPartitionBl;
    uint32_t bootArgsStr;
    uint32_t whitelistNvram;
    uint32_t whitelist;
    uint32_t newlist;
    size_t   whitelistLen;
    uint32_t envSetBootPartition;
    
#ifdef HAVE_IBOOT_EXPLOIT
    uint32_t hookPoint;
    uint32_t mountAndBootSystem;
    uint32_t shcStart;
#endif
    
#ifdef N42
    /*
     * Apple Root CA
     * 0x47840 // new whitelist nvram
     * 0x47940 // shellcode
     * 0x47b00 // bootArgsStr
     *
     */
    
    rsa_patch           = 0x1621A;
    debug_patch         = 0x16B62;
    bootArgs_normal     = 0x17AA0;
    bootArgs_rdsk1      = 0x17AA4;
    bootArgs_rdsk2      = 0x17AA8;
    bootmode            = 0x1E1B0;
    goCmd               = 0x45FDC;
    bootRamdisk         = 0x00F4C;
    bootRamdiskStr      = 0x3CA88;
    bootPartition       = 0x3CA5C;
    bootPartitionStr    = 0x3CA61;
    bootPartitionBl     = 0x00E3C;
    whitelistNvram      = 0x461FC;
    whitelist           = 0x0129C;
    whitelistLen        = 0x00038;
    envSetBootPartition = 0x01a8e;
    newlist             = 0x47840;
    bootArgsStr         = 0x47B00;
    
#ifdef HAVE_IBOOT_EXPLOIT
    hookPoint           = 0x00bb0;
    mountAndBootSystem  = 0x00ef0;
    shcStart            = 0x47940;
#endif
    
    if(open_file(path, &size, &buf) != 0){
        printf("[%s] ERROR: Failed to get image: %s!\n", __FUNCTION__, path);
        return;
    }
    
    printf("[%s] Apply rsa_patch: 0x%08x\n", __FUNCTION__, rsa_patch);
    *(uint32_t*)(buf+IBOOT_DATA_START+rsa_patch)        = 0x60182000;
    
    printf("[%s] Apply debug_patch: 0x%08x\n", __FUNCTION__, debug_patch);
    *(uint32_t*)(buf+IBOOT_DATA_START+debug_patch)      = 0x20012001;
    
    printf("[%s] Apply bootArgs_normal patch: 0x%08x\n", __FUNCTION__, bootArgs_normal);
    *(uint32_t*)(buf+IBOOT_DATA_START+bootArgs_normal)  = A6_IBOOT_IMAGE_BASE + bootArgsStr + 7;
    
    printf("[%s] Apply bootArgs_rdsk1 patch: 0x%08x\n", __FUNCTION__, bootArgs_rdsk1);
    *(uint32_t*)(buf+IBOOT_DATA_START+bootArgs_rdsk1)   = A6_IBOOT_IMAGE_BASE + bootArgsStr;
    
    printf("[%s] Apply bootArgs_rdsk2 patch: 0x%08x\n", __FUNCTION__, bootArgs_rdsk2);
    *(uint32_t*)(buf+IBOOT_DATA_START+bootArgs_rdsk2)   = A6_IBOOT_IMAGE_BASE + bootArgsStr;
    
#ifdef HAVE_IBOOT_EXPLOIT
    printf("[%s] Apply bootmode patch: 0x%08x\n", __FUNCTION__, bootmode);
    *(uint32_t*)(buf+IBOOT_DATA_START+bootmode)         = 0x47702000;
#else
    printf("[%s] Apply bootmode patch: 0x%08x\n", __FUNCTION__, bootmode);
    *(uint32_t*)(buf+IBOOT_DATA_START+bootmode)         = 0x47702001;
    
    printf("[%s] Apply goCmd patch: 0x%08x\n", __FUNCTION__, goCmd);
    *(uint32_t*)(buf+IBOOT_DATA_START+goCmd)            = A6_IBOOT_LOADADDR_BASE;
#endif
    
    printf("[%s] Apply boot-ramdisk patch: 0x%08x\n", __FUNCTION__, bootRamdisk);
    *(uint32_t*)(buf+IBOOT_DATA_START+bootRamdisk)      = 0x20002000;
    
    printf("[%s] Apply boot-partition patch: 0x%08x\n", __FUNCTION__, bootPartitionBl);
    *(uint32_t*)(buf+IBOOT_DATA_START+bootPartitionBl)  = 0x20002000;
    
    printf("[%s] Apply whitelist patch: 0x%08x\n", __FUNCTION__, whitelist);
    *(uint32_t*)(buf+IBOOT_DATA_START+whitelist)        = A6_IBOOT_IMAGE_BASE + newlist;
    
    printf("[%s] Apply new whitelist patch: 0x%08x\n", __FUNCTION__, newlist);
    memcpy((void*)(buf+IBOOT_DATA_START+newlist), (void*)(buf+IBOOT_DATA_START+whitelistNvram), whitelistLen);
    
    /*
     * env_blacklist_nvram function: whitelist
     *  Only variables allowed by this routine can be read for nvram values using env_get().
     *
     *   whitelist[] = {
     *    "auto-boot",
     *    "backlight-level",
     *    "boot-command",
     *    "com.apple.System.boot-nonce",
     *    "debug-uarts",
     *    "device-material",
     *    "display-rotation",
     *    "idle-off",
     *    "is-tethered",
     *    "darkboot",
     *    "ota-breadcrumbs",
     *    "boot-breadcrumbs",
     *    "com.apple.System.tz0-size",
     *    "pwr-path",
     *    NULL
     *   };
     */
    unsigned int val = IBOOT_DATA_START+newlist+whitelistLen;
    *(uint32_t*)(buf+val) = A6_IBOOT_IMAGE_BASE + bootRamdiskStr;   val+=4; // add "boot-ramdisk"
    *(uint32_t*)(buf+val) = A6_IBOOT_IMAGE_BASE + bootPartitionStr; val+=4; // add "boot-partition"
    *(uint32_t*)(buf+val) = 0;                                              // end
    
    /*
     * NOP out the call to env_set("boot-partition", "0", 0) to prevent the value of boot-partition from being reset to "0" on sys_setup_default_environment().
     */
    printf("[%s] Apply env-set boot-partition patch: 0x%08x\n", __FUNCTION__, envSetBootPartition);
    *(uint16_t*)(buf+IBOOT_DATA_START+envSetBootPartition)  = 0xbf00;
    
    
    printf("[%s] New bootArgs: \"%s\"\n", __FUNCTION__, str);
    printf("[%s] Apply bootArgsStr patch: 0x%08x\n", __FUNCTION__, bootArgsStr);
    strcpy((char *)(buf+IBOOT_DATA_START+bootArgsStr), str);
    
    
    
#ifdef HAVE_IBOOT_EXPLOIT
    
    /* shellcode */
    
    printf("[%s] Apply hook_point patch: 0x%08x\n", __FUNCTION__, hookPoint);
    *(uint32_t*)(buf+IBOOT_DATA_START+hookPoint)            = 0xea26f4ff; // blx        PAYLOAD_BASE
    
    printf("[%s] Apply mount_and_boot_system patch: 0x%08x\n", __FUNCTION__, mountAndBootSystem);
    *(uint32_t*)(buf+IBOOT_DATA_START+mountAndBootSystem)   = 0xbd26f046; // b.w        _shellcode
    
    {
        int i=0;
        printf("[%s] Apply shellcode patch: 0x%08x\n", __FUNCTION__, shcStart);
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0x2000;     i+=2; // movs       r0, #0x0
        *(uint32_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0x1180f44f; i+=4; // mov.w      r1, #0x100000
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0x4a05;     i+=2; // ldr        r2, =IMAGE3_SIZE
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0x6010;     i+=2; // str        r0, [r2]
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0x4805;     i+=2; // ldr        r0, =PAYLOAD_BASE
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0x4b05;     i+=2; // ldr        r3, =IMAGE_PYLD_TYPE
        *(uint32_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0xfde9f7d4; i+=4; // bl         _image_load_type
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0xb5f0;     i+=2; // push       {r4, r5, r6, r7, lr}
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0xaf03;     i+=2; // add        r7, sp, #0xc
        *(uint32_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0xbacdf7b9; i+=4; // b.w        _mount_and_boot_system+4
        *(uint16_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0xbf00;     i+=2; // nop
        *(uint32_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0xbff47af0; i+=4; // :: IMAGE3_SIZE
        *(uint32_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0xbfc00000; i+=4; // :: PAYLOAD_BASE
        *(uint32_t*)(buf+IBOOT_DATA_START+shcStart+i) = 0x646c7970; i+=4; // :: IMAGE_PYLD_TYPE
    }
    
    printf("[%s] Apply IMAGE_TYPE patch: 0x%08x, 0x%08x\n", __FUNCTION__, 0x10, 0x20);
    *(uint32_t*)(buf+0x10) = 0x69626f62;
    *(uint32_t*)(buf+0x20) = 0x69626f62;
    
#endif
    
    FILE *out = fopen(path, "w");
    if (!out) {
        printf("[%s] ERROR: Failed to open image: %s!\n", __FUNCTION__, path);
        return;
    }
    
    fwrite(buf, size, 1, out);
    fflush(out);
    fclose(out);
    
    free(buf);
    
    return;
    
#endif
    
}

int main(void)
{

#ifdef N42
    patch_iboot("fw/Firmware/all_flash/all_flash.n42ap.production/iBootX.n42ap.RELEASE.img3");
#endif
    
    return 0;
}
