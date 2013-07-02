
//
//    Copyright (C) 2013 Ian McMahon
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
//


#include <asm/io.h>

#include "rtapi.h"
#include "rtapi_app.h"
#include "rtapi_math.h"
#include "rtapi_string.h"

#include "hal.h"

#include "hal/drivers/mesa-hostmot2/bitfile.h"
#include "hal/drivers/mesa-hostmot2/hostmot2-lowlevel.h"
#include "hal/drivers/mesa-hostmot2/hm2_bcc.h"


static int comp_id;

#ifdef MODULE_INFO
MODULE_INFO(linuxcnc, "component:hm2_bcc:LinuxCNC HAL driver for the BeagleBone/Blank Canvas Cape, with HostMot2 firmware.");
MODULE_INFO(linuxcnc, "license:GPL");
#endif // MODULE_INFO

MODULE_LICENSE("GPL");


static char *config[HM2_BCC_MAX_BOARDS];
static int num_config_strings = HM2_BCC_MAX_BOARDS;
module_param_array(config, charp, &num_config_strings, S_IRUGO);
MODULE_PARM_DESC(config, "config string(s) for the BCC board(s) (see hostmot2(9) manpage)");




//
// this data structure keeps track of all the 7i43 boards found
//

static hm2_7i43_t board[HM2_7I43_MAX_BOARDS];
static int num_boards;



// 
// these are the low-level i/o functions exported to the hostmot2 driver
//

int hm2_bcc_read(hm2_lowlevel_io_t *this, u32 addr, void *buffer, int size) {
    int bytes_remaining = size;
    hm2_bcc_t *board = this->private;


    return 1;  // success
}




int hm2_7i43_write(hm2_lowlevel_io_t *this, u32 addr, void *buffer, int size) {
    int bytes_remaining = size;
    hm2_7i43_t *board = this->private;

    return 1;
}




int hm2_bcc_program_fpga(hm2_lowlevel_io_t *this, const bitfile_t *bitfile) {
    hm2_bcc_t *board = this->private;
    int64_t start_time, end_time;
    int i;
    const u8 *firmware = bitfile->e.data;


    //
    // send the firmware
    //

    start_time = rtapi_get_time();

    end_time = rtapi_get_time();

    //
    // brag about how fast it was
    //

    {
        uint32_t duration_ns;

        duration_ns = (uint32_t)(end_time - start_time);

        if (duration_ns != 0) {
            THIS_INFO(
                "%d bytes of firmware sent (%u KB/s)\n",
                bitfile->e.size,
                (uint32_t)(((double)bitfile->e.size / ((double)duration_ns / (double)(1000 * 1000 * 1000))) / 1024)
            );
        }
    }


    return 0;
}




// return 0 if the board has been reset, -errno if not
int hm2_bcc_reset(hm2_lowlevel_io_t *this) {
    hm2_bcc_t *board = this->private;
    uint8_t byte;

    return 0;
}




//
// setup and cleanup code
//


static void hm2_bcc_cleanup(void) {
    int i;

    // NOTE: hal_malloc() doesnt have a matching free

    for (i = 0; i < num_boards; i ++) {
        hm2_lowlevel_io_t *this = &board[i].llio;
        THIS_PRINT("releasing board\n");
        hm2_unregister(this);
        hal_parport_release(&board[i].port);
    }
}


static int hm2_bcc_setup(void) {
    int i;

    LL_PRINT("loading HostMot2 BCC driver version %s\n", HM2_BCC_VERSION);

    // zero the board structs
    memset(board, 0, HM2_BCC_MAX_BOARDS * sizeof(hm2_7i43_t));
    num_boards = 0;

    for (i = 0; i < num_config_strings; i ++) {
        hm2_lowlevel_io_t *this;
        int r;

        rtapi_snprintf(board[i].llio.name, sizeof(board[i].llio.name), "%s.%d", HM2_LLIO_NAME, i);
        board[i].llio.comp_id = comp_id;

        board[i].llio.read = hm2_bcc_read;
        board[i].llio.write = hm2_bcc_write;
        board[i].llio.program_fpga = hm2_bcc_program_fpga;
        board[i].llio.reset = hm2_bcc_reset;

        board[i].llio.num_ioport_connectors = 1;
        board[i].llio.pins_per_connector = 24;
        board[i].llio.ioport_connector_name[0] = "J201";
        board[i].llio.num_leds = 2;
        board[i].llio.private = &board[i];

		  board[i].llio.fpga_part_number = "3s200alqfp100";

        this = &board[i].llio;

        r = hm2_register(&board[i].llio, config[i]);
        if (r != 0) {
            THIS_ERR(
                "board at (spi/i2c/gpmc address details go here) not found!\n",
            );

            hal_parport_release(&board[i].port);
            return r;
        }

        THIS_PRINT(
            "board at (spi/i2c/gpmc address details go here) found\n",
        );

        num_boards ++;
    }

    return 0;
}


int rtapi_app_main(void) {
    int r = 0;

    comp_id = hal_init(HM2_LLIO_NAME);
    if (comp_id < 0) return comp_id;

    r = hm2_bcc_setup();
    if (r) {
        hm2_bcc_cleanup();
        hal_exit(comp_id);
    } else {
        hal_ready(comp_id);
    }

    return r;
}


void rtapi_app_exit(void) {
    hm2_bcc_cleanup();
    hal_exit(comp_id);
    LL_PRINT("driver unloaded\n");
}

