
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

#define HM2_BCC_VERSION "0.1"

#define HM2_LLIO_NAME "hm2_bcc"


#define HM2_BCC_MAX_BOARDS (4)


//
// The BCC struct
//

typedef struct {
	// this could potentially contain details such as the spidev it's attached to, the i2c address of the config mux, or gpmc details.

    hm2_lowlevel_io_t llio;
} hm2_bcc_t;

