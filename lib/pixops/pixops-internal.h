/*
 * Copyright (C) 2000 Red Hat, Inc
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifdef USE_MMX
guchar * pixops_scale_line_22_33_mmx (guint32 weights[16][8], guchar *p, guchar *q1, guchar *q2, int x_step, guchar *p_stop, int x_init);
guchar * pixops_composite_line_22_4a4_mmx (guint32 weights[16][8], guchar *p, guchar *q1, guchar *q2, int x_step, guchar *p_stop, int x_init);
guchar * pixops_composite_line_color_22_4a4_mmx (guint32 weights[16][8], guchar *p, guchar *q1, guchar *q2, int x_step, guchar *p_stop, int x_init, int dest_x, int check_shift, int *colors);
int  pixops_have_mmx (void);
#endif

