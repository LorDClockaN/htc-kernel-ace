/* drivers/video/msm_fb/mdp_hw.h
 *
 * Copyright (C) 2007 QUALCOMM Incorporated
 * Copyright (C) 2007 Google Incorporated
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef _MDP_HW_H_
#define _MDP_HW_H_

#include <linux/platform_device.h>
#include <linux/wait.h>
#include <mach/msm_iomap.h>
#include <mach/msm_fb.h>

typedef void (*mdp_dma_start_func_t)(void *private_data, uint32_t addr,
				     uint32_t stride, uint32_t width,
				     uint32_t height, uint32_t x, uint32_t y);

struct mdp_out_interface {
	uint32_t		registered:1;
	void			*priv;

	/* If the interface client wants to get DMA_DONE events */
	uint32_t		dma_mask;
	mdp_dma_start_func_t	dma_start;

	struct msmfb_callback	*dma_cb;
	wait_queue_head_t	dma_waitqueue;

	/* If the interface client wants to be notified of non-DMA irqs,
	 * e.g. LCDC/TV-out frame start */
	uint32_t		irq_mask;
	struct msmfb_callback	*irq_cb;
};

struct mdp_info {
	spinlock_t lock;
	struct mdp_device mdp_dev;
	char * __iomem base;
	int irq;
	struct clk *clk;
	struct clk *pclk;
	struct clk *ebi1_clk;
	struct mdp_out_interface out_if[MSM_MDP_NUM_INTERFACES];
	int dma_format;
	int dma_pack_pattern;
	bool dma_config_dirty;
	struct mdp_blit_req *req;
	uint32_t state;
	struct timer_list standby_timer;
	struct timer_list dma_timer;

	int (*enable_irq)(struct mdp_info *mdp, uint32_t mask);
	int (*disable_irq)(struct mdp_info *mdp, uint32_t mask);
	int (*write_regs)(struct mdp_info *mdp, const struct mdp_reg *reglist, int size);
};

struct mdp_lcdc_info {
	struct mdp_info			*mdp;
	struct clk			*mdp_clk;
	struct clk			*pclk;
	struct clk			*pad_pclk;
	struct msm_panel_data		fb_panel_data;
	struct platform_device		fb_pdev;
	struct msm_lcdc_platform_data	*pdata;
	uint32_t fb_start;

	struct msmfb_callback		frame_start_cb;
	wait_queue_head_t		vsync_waitq;
	int				got_vsync;
	unsigned			color_format;
	struct {
		uint32_t	clk_rate;
		uint32_t	hsync_ctl;
		uint32_t	vsync_period;
		uint32_t	vsync_pulse_width;
		uint32_t	display_hctl;
		uint32_t	display_vstart;
		uint32_t	display_vend;
		uint32_t	hsync_skew;
		uint32_t	polarity;
	} parms;
};

struct panel_icm_info {
	bool	icm_mode;
	bool	icm_doable;
	bool	clock_enabled;
	int	panel_update;
	bool	icm_suspend;
	struct mutex icm_lock;
	struct mdp_lcdc_info *lcdc;
	spinlock_t lock;
	void (*force_leave)(void);
};

extern int mdp_out_if_register(struct mdp_device *mdp_dev, int interface,
			       void *private_data, uint32_t dma_mask,
			       mdp_dma_start_func_t dma_start);

extern int mdp_out_if_req_irq(struct mdp_device *mdp_dev, int interface,
			      uint32_t mask, struct msmfb_callback *cb);

struct mdp_blit_req;
struct mdp_device;

void mdp_ppp_dump_debug(const struct mdp_info *mdp);
int mdp_hw_init(struct mdp_info *mdp);
void mdp_check_tearing(struct mdp_info *mdp, struct msm_mdp_platform_data *pdata);
void mdp_dump_blit(struct mdp_blit_req *req);
int mdp_wait(struct mdp_info *mdp, uint32_t mask, wait_queue_head_t *wq);

#define mdp_writel(mdp, value, offset) writel(value, mdp->base + offset)
#define mdp_readl(mdp, offset) readl(mdp->base + offset)
#define panel_to_lcdc(p) container_of((p), struct mdp_lcdc_info, fb_panel_data)

/* define mdp state for multi purpose */
#define MDP_STATE_STANDBY		(1 << 0)


#ifdef CONFIG_MSM_MDP302
#define MDP_SYNC_CONFIG_0                ( 0x00300)
#define MDP_SYNC_CONFIG_1                ( 0x00304)
#define MDP_SYNC_CONFIG_2                ( 0x00308)
#else
#define MDP_SYNC_CONFIG_0                ( 0x00000)
#define MDP_SYNC_CONFIG_1                ( 0x00004)
#define MDP_SYNC_CONFIG_2                ( 0x00008)
#endif

#define MDP_SYNC_STATUS_0                ( 0x0000c)
#define MDP_SYNC_STATUS_1                ( 0x00010)
#define MDP_SYNC_STATUS_2                ( 0x00014)

#ifdef CONFIG_MSM_MDP302
#define MDP_SYNC_THRESH_0                ( 0x00200)
#define MDP_SYNC_THRESH_1                ( 0x00204)
#else
#define MDP_SYNC_THRESH_0                ( 0x00018)
#define MDP_SYNC_THRESH_1                ( 0x0001c)
#endif
#ifdef CONFIG_MSM_MDP40
#define MDP_INTR_ENABLE                  ( 0x0050)
#define MDP_INTR_STATUS                  ( 0x0054)
#define MDP_INTR_CLEAR                   ( 0x0058)
#define MDP_EBI2_LCD0                    ( 0x0060)
#define MDP_EBI2_LCD1                    ( 0x0064)
#define MDP_EBI2_PORTMAP_MODE            ( 0x0070)

#define MDP_DMA_P_HIST_INTR_STATUS 	( 0x95014)
#define MDP_DMA_P_HIST_INTR_CLEAR 	( 0x95018)
#define MDP_DMA_P_HIST_INTR_ENABLE 	( 0x9501C)
#else
#define MDP_INTR_ENABLE                  ( 0x00020)
#define MDP_INTR_STATUS                  ( 0x00024)
#define MDP_INTR_CLEAR                   ( 0x00028)
#define MDP_EBI2_LCD0                    ( 0x0003c)
#define MDP_EBI2_LCD1                    ( 0x00040)
#define MDP_EBI2_PORTMAP_MODE            ( 0x0005c)
#endif
#define MDP_DISPLAY0_START               ( 0x00030)
#define MDP_DISPLAY1_START               ( 0x00034)
#define MDP_DISPLAY_STATUS               ( 0x00038)
/* CONFIG_MSM_MDP302 */
#define MDP_TEAR_CHECK_EN                ( 0x0020c)
#define MDP_PRIM_START_POS               ( 0x00210)

#ifndef CONFIG_MSM_MDP31
#define MDP_DISPLAY0_ADDR                (0x00054)
#define MDP_DISPLAY1_ADDR                (0x00058)
#define MDP_PPP_CMD_MODE                 (0x00060)
#else
#define MDP_DISPLAY0_ADDR                (0x10000)
#define MDP_DISPLAY1_ADDR                (0x10004)
#define MDP_PPP_CMD_MODE                 (0x10060)
#endif

#define MDP_TV_OUT_STATUS                (0x00064)
#define MDP_HW_VERSION                   (0x00070)
#define MDP_SW_RESET                     (0x00074)
#define MDP_AXI_ERROR_MASTER_STOP        (0x00078)
#define MDP_SEL_CLK_OR_HCLK_TEST_BUS     (0x0007c)
#define MDP_PRIMARY_VSYNC_OUT_CTRL       (0x00080)
#define MDP_SECONDARY_VSYNC_OUT_CTRL     (0x00084)
#define MDP_EXTERNAL_VSYNC_OUT_CTRL      (0x00088)
#define MDP_VSYNC_CTRL                   (0x0008c)
#define MDP_MDDI_PARAM_WR_SEL            (0x00090)
#define MDP_MDDI_PARAM                   (0x00094)
#define MDP_MDDI_DATA_XFR                (0x00098)


#if defined(CONFIG_MSM_MDP40)
#define MDP_LAYERMIXER_IN_CFG            (0x10100)
#define MDP_OVERLAYPROC0_CFG             (0x10004)
#define MDP_OVERLAYPROC1_CFG             (0x18004)
#endif
#define MDP_CGC_EN                       (0x00100)
#define MDP_CMD_STATUS                   (0x10008)
#define MDP_PROFILE_EN                   (0x10010)
#define MDP_PROFILE_COUNT                (0x10014)
#define MDP_DMA_START                    (0x10044)
#define MDP_FULL_BYPASS_WORD0            (0x10100)
#define MDP_FULL_BYPASS_WORD1            (0x10104)
#define MDP_COMMAND_CONFIG               (0x10104)
#define MDP_FULL_BYPASS_WORD2            (0x10108)
#define MDP_FULL_BYPASS_WORD3            (0x1010c)
#define MDP_FULL_BYPASS_WORD4            (0x10110)
#define MDP_FULL_BYPASS_WORD6            (0x10118)
#define MDP_FULL_BYPASS_WORD7            (0x1011c)
#define MDP_FULL_BYPASS_WORD8            (0x10120)
#define MDP_FULL_BYPASS_WORD9            (0x10124)
#define MDP_PPP_SOURCE_CONFIG            (0x10124)
#define MDP_FULL_BYPASS_WORD10           (0x10128)
#define MDP_FULL_BYPASS_WORD11           (0x1012c)
#define MDP_FULL_BYPASS_WORD12           (0x10130)
#define MDP_FULL_BYPASS_WORD13           (0x10134)
#define MDP_FULL_BYPASS_WORD14           (0x10138)
#define MDP_PPP_OPERATION_CONFIG         (0x10138)
#define MDP_FULL_BYPASS_WORD15           (0x1013c)
#define MDP_FULL_BYPASS_WORD16           (0x10140)
#define MDP_FULL_BYPASS_WORD17           (0x10144)
#define MDP_FULL_BYPASS_WORD18           (0x10148)
#define MDP_FULL_BYPASS_WORD19           (0x1014c)
#define MDP_FULL_BYPASS_WORD20           (0x10150)
#define MDP_PPP_DESTINATION_CONFIG       (0x10150)
#define MDP_FULL_BYPASS_WORD21           (0x10154)
#define MDP_FULL_BYPASS_WORD22           (0x10158)
#define MDP_FULL_BYPASS_WORD23           (0x1015c)
#define MDP_FULL_BYPASS_WORD24           (0x10160)
#define MDP_FULL_BYPASS_WORD25           (0x10164)
#define MDP_FULL_BYPASS_WORD26           (0x10168)
#define MDP_FULL_BYPASS_WORD27           (0x1016c)
#define MDP_FULL_BYPASS_WORD29           (0x10174)
#define MDP_FULL_BYPASS_WORD30           (0x10178)
#define MDP_FULL_BYPASS_WORD31           (0x1017c)
#define MDP_FULL_BYPASS_WORD32           (0x10180)
#define MDP_DMA_CONFIG                   (0x10180)
#define MDP_FULL_BYPASS_WORD33           (0x10184)
#define MDP_FULL_BYPASS_WORD34           (0x10188)
#define MDP_FULL_BYPASS_WORD35           (0x1018c)
#define MDP_FULL_BYPASS_WORD37           (0x10194)
#define MDP_FULL_BYPASS_WORD39           (0x1019c)
#define MDP_PPP_OUT_XY                   (0x1019c)
#define MDP_FULL_BYPASS_WORD40           (0x101a0)
#define MDP_FULL_BYPASS_WORD41           (0x101a4)
#define MDP_FULL_BYPASS_WORD43           (0x101ac)
#define MDP_FULL_BYPASS_WORD46           (0x101b8)
#define MDP_FULL_BYPASS_WORD47           (0x101bc)
#define MDP_FULL_BYPASS_WORD48           (0x101c0)
#define MDP_FULL_BYPASS_WORD49           (0x101c4)
#define MDP_FULL_BYPASS_WORD50           (0x101c8)
#define MDP_FULL_BYPASS_WORD51           (0x101cc)
#define MDP_FULL_BYPASS_WORD52           (0x101d0)
#define MDP_FULL_BYPASS_WORD53           (0x101d4)
#define MDP_FULL_BYPASS_WORD54           (0x101d8)
#define MDP_FULL_BYPASS_WORD55           (0x101dc)
#define MDP_FULL_BYPASS_WORD56           (0x101e0)
#define MDP_FULL_BYPASS_WORD57           (0x101e4)
#define MDP_FULL_BYPASS_WORD58           (0x101e8)
#define MDP_FULL_BYPASS_WORD59           (0x101ec)
#define MDP_FULL_BYPASS_WORD60           (0x101f0)
#define MDP_VSYNC_THRESHOLD              (0x101f0)
#define MDP_FULL_BYPASS_WORD61           (0x101f4)
#define MDP_FULL_BYPASS_WORD62           (0x101f8)
#define MDP_FULL_BYPASS_WORD63           (0x101fc)

#ifdef CONFIG_MSM_MDP31
#define MDP_PPP_SRC_XY                   (0x10200)
#define MDP_PPP_BG_XY                    (0x10204)
#define MDP_PPP_SRC_IMAGE_SIZE           (0x10208)
#define MDP_PPP_BG_IMAGE_SIZE            (0x1020c)
#define MDP_PPP_SCALE_CONFIG             (0x10230)
#define MDP_PPP_CSC_CONFIG               (0x10240)
#define MDP_PPP_BLEND_BG_ALPHA_SEL       (0x70010)
#endif

#define MDP_TFETCH_TEST_MODE             (0x20004)
#define MDP_TFETCH_STATUS                (0x20008)
#define MDP_TFETCH_TILE_COUNT            (0x20010)
#define MDP_TFETCH_FETCH_COUNT           (0x20014)
#define MDP_TFETCH_CONSTANT_COLOR        (0x20040)
#define MDP_BGTFETCH_TEST_MODE           (0x28004)
#define MDP_BGTFETCH_STATUS              (0x28008)
#define MDP_BGTFETCH_TILE_COUNT          (0x28010)
#define MDP_BGTFETCH_FETCH_COUNT         (0x28014)
#define MDP_BGTFETCH_CONSTANT_COLOR      (0x28040)
#define MDP_CSC_BYPASS                   (0x40004)
#define MDP_SCALE_COEFF_LSB              (0x5fffc)
#define MDP_TV_OUT_CTL                   (0xc0000)
#define MDP_TV_OUT_FIR_COEFF             (0xc0004)
#define MDP_TV_OUT_BUF_ADDR              (0xc0008)
#define MDP_TV_OUT_CC_DATA               (0xc000c)
#define MDP_TV_OUT_SOBEL                 (0xc0010)
#define MDP_TV_OUT_Y_CLAMP               (0xc0018)
#define MDP_TV_OUT_CB_CLAMP              (0xc001c)
#define MDP_TV_OUT_CR_CLAMP              (0xc0020)
#define MDP_TEST_MODE_CLK                (0xd0000)
#define MDP_TEST_MISR_RESET_CLK          (0xd0004)
#define MDP_TEST_EXPORT_MISR_CLK         (0xd0008)
#define MDP_TEST_MISR_CURR_VAL_CLK       (0xd000c)
#define MDP_TEST_MODE_HCLK               (0xd0100)
#define MDP_TEST_MISR_RESET_HCLK         (0xd0104)
#define MDP_TEST_EXPORT_MISR_HCLK        (0xd0108)
#define MDP_TEST_MISR_CURR_VAL_HCLK      (0xd010c)
#define MDP_TEST_MODE_DCLK               (0xd0200)
#define MDP_TEST_MISR_RESET_DCLK         (0xd0204)
#define MDP_TEST_EXPORT_MISR_DCLK        (0xd0208)
#define MDP_TEST_MISR_CURR_VAL_DCLK      (0xd020c)
#define MDP_TEST_CAPTURED_DCLK           (0xd0210)
#define MDP_TEST_MISR_CAPT_VAL_DCLK      (0xd0214)

#ifdef CONFIG_MSM_MDP40
#define MDP_DMA_P_START                  (0x000c)
#else
#define MDP_DMA_P_START                  (0x00044)
#endif
#define MDP_DMA_P_CONFIG                 (0x90000)
#define MDP_DMA_P_SIZE                   (0x90004)
#define MDP_DMA_P_IBUF_ADDR              (0x90008)
#define MDP_DMA_P_IBUF_Y_STRIDE          (0x9000c)
#define MDP_DMA_P_OUT_XY                 (0x90010)
#define MDP_DMA_P_COLOR_CORRECT_CONFIG   (0x90070)

#define MDP_DMA_S_START                  (0x00048)
#define MDP_DMA_S_CONFIG                 (0xa0000)
#define MDP_DMA_S_SIZE                   (0xa0004)
#define MDP_DMA_S_IBUF_ADDR              (0xa0008)
#define MDP_DMA_S_IBUF_Y_STRIDE          (0xa000c)
#define MDP_DMA_S_OUT_XY                 (0xa0010)

#ifdef CONFIG_MSM_MDP40
#define MDP_LCDC_EN                      (0xc0000)
#define MDP_LCDC_HSYNC_CTL               (0xc0004)
#define MDP_LCDC_VSYNC_PERIOD            (0xc0008)
#define MDP_LCDC_VSYNC_PULSE_WIDTH       (0xc000c)
#define MDP_LCDC_DISPLAY_HCTL            (0xc0010)
#define MDP_LCDC_DISPLAY_V_START         (0xc0014)
#define MDP_LCDC_DISPLAY_V_END           (0xc0018)
#define MDP_LCDC_ACTIVE_HCTL             (0xc001c)
#define MDP_LCDC_ACTIVE_V_START          (0xc0020)
#define MDP_LCDC_ACTIVE_V_END            (0xc0024)
#define MDP_LCDC_BORDER_CLR              (0xc0028)
#define MDP_LCDC_UNDERFLOW_CTL           (0xc002c)
#define MDP_LCDC_HSYNC_SKEW              (0xc0030)
#define MDP_LCDC_TEST_CTL                (0xc0034)
#define MDP_LCDC_CTL_POLARITY            (0xc0038)
#else
#define MDP_LCDC_EN                      (0xe0000)
#define MDP_LCDC_HSYNC_CTL               (0xe0004)
#define MDP_LCDC_VSYNC_PERIOD            (0xe0008)
#define MDP_LCDC_VSYNC_PULSE_WIDTH       (0xe000c)
#define MDP_LCDC_DISPLAY_HCTL            (0xe0010)
#define MDP_LCDC_DISPLAY_V_START         (0xe0014)
#define MDP_LCDC_DISPLAY_V_END           (0xe0018)
#define MDP_LCDC_ACTIVE_HCTL             (0xe001c)
#define MDP_LCDC_ACTIVE_V_START          (0xe0020)
#define MDP_LCDC_ACTIVE_V_END            (0xe0024)
#define MDP_LCDC_BORDER_CLR              (0xe0028)
#define MDP_LCDC_UNDERFLOW_CTL           (0xe002c)
#define MDP_LCDC_HSYNC_SKEW              (0xe0030)
#define MDP_LCDC_TEST_CTL                (0xe0034)
#define MDP_LCDC_CTL_POLARITY            (0xe0038)
#endif

#define MDP_PPP_SCALE_STATUS             (0x50000)
#define MDP_PPP_BLEND_STATUS             (0x70000)

/* MDP_SW_RESET */
#define MDP_PPP_SW_RESET                (1<<4)

/* MDP_INTR_ENABLE */
#define DL0_ROI_DONE			(1<<0)
#define TV_OUT_DMA3_DONE		(1<<6)
#define TV_ENC_UNDERRUN			(1<<7)
#define TV_OUT_FRAME_START		(1<<13)

#ifdef CONFIG_MSM_MDP22
#define MDP_DMA_P_DONE			(1 << 2)
#define MDP_DMA_S_DONE			(1 << 3)
#else /* CONFIG_MSM_MDP31 */

#ifdef CONFIG_MSM_MDP40
#define MDP_DMA_P_DONE			(1 << 4)
#else
#define MDP_DMA_P_DONE			(1 << 14)
#endif

#define MDP_DMA_S_DONE			(1 << 2)
#define MDP_LCDC_UNDERFLOW		(1 << 16)

#ifdef CONFIG_MSM_MDP40
#define MDP_LCDC_FRAME_START		(1 << 7)
#else
#define MDP_LCDC_FRAME_START		(1 << 15)
#endif

#endif

#define MDP_TOP_LUMA       16
#define MDP_TOP_CHROMA     0
#define MDP_BOTTOM_LUMA    19
#define MDP_BOTTOM_CHROMA  3
#define MDP_LEFT_LUMA      22
#define MDP_LEFT_CHROMA    6
#define MDP_RIGHT_LUMA     25
#define MDP_RIGHT_CHROMA   9

#define CLR_G 0x0
#define CLR_B 0x1
#define CLR_R 0x2
#define CLR_ALPHA 0x3

#define CLR_Y  CLR_G
#define CLR_CB CLR_B
#define CLR_CR CLR_R

/* from lsb to msb */
#define MDP_GET_PACK_PATTERN(a, x, y, z, bit) \
	(((a)<<(bit*3))|((x)<<(bit*2))|((y)<<bit)|(z))

/* MDP_SYNC_CONFIG_0/1/2 */
#if defined(CONFIG_MSM_MDP30)
#define MDP_SYNCFG_HGT_LOC 21
#define MDP_SYNCFG_VSYNC_EXT_EN (1<<20)
#define MDP_SYNCFG_VSYNC_INT_EN (1<<19)
#else
#define MDP_SYNCFG_HGT_LOC 22
#define MDP_SYNCFG_VSYNC_EXT_EN (1<<21)
#define MDP_SYNCFG_VSYNC_INT_EN (1<<20)
#endif

/* MDP_SYNC_THRESH_0 */
#define MDP_PRIM_BELOW_LOC 0
#define MDP_PRIM_ABOVE_LOC 8

/* MDP_{PRIMARY,SECONDARY,EXTERNAL}_VSYNC_OUT_CRL */
#define VSYNC_PULSE_EN (1<<31)
#define VSYNC_PULSE_INV (1<<30)

/* MDP_VSYNC_CTRL */
#define DISP0_VSYNC_MAP_VSYNC0 0
#define DISP0_VSYNC_MAP_VSYNC1 (1<<0)
#define DISP0_VSYNC_MAP_VSYNC2 (1<<0)|(1<<1)

#define DISP1_VSYNC_MAP_VSYNC0 0
#define DISP1_VSYNC_MAP_VSYNC1 (1<<2)
#define DISP1_VSYNC_MAP_VSYNC2 (1<<2)|(1<<3)

#define PRIMARY_LCD_SYNC_EN (1<<4)
#define PRIMARY_LCD_SYNC_DISABLE 0

#define SECONDARY_LCD_SYNC_EN (1<<5)
#define SECONDARY_LCD_SYNC_DISABLE 0

#define EXTERNAL_LCD_SYNC_EN (1<<6)
#define EXTERNAL_LCD_SYNC_DISABLE 0

/* MDP_VSYNC_THRESHOLD / MDP_FULL_BYPASS_WORD60 */
#define VSYNC_THRESHOLD_ABOVE_LOC 0
#define VSYNC_THRESHOLD_BELOW_LOC 16
#define VSYNC_ANTI_TEAR_EN (1<<31)

/* MDP_COMMAND_CONFIG / MDP_FULL_BYPASS_WORD1 */
#define MDP_CMD_DBGBUS_EN (1<<0)

/* MDP_PPP_SOURCE_CONFIG / MDP_FULL_BYPASS_WORD9&53 */
#define PPP_SRC_C0G_8BIT ((1<<1)|(1<<0))
#define PPP_SRC_C1B_8BIT ((1<<3)|(1<<2))
#define PPP_SRC_C2R_8BIT ((1<<5)|(1<<4))
#define PPP_SRC_C3A_8BIT ((1<<7)|(1<<6))

#define PPP_SRC_C0G_6BIT (1<<1)
#define PPP_SRC_C1B_6BIT (1<<3)
#define PPP_SRC_C2R_6BIT (1<<5)

#define PPP_SRC_C0G_5BIT (1<<0)
#define PPP_SRC_C1B_5BIT (1<<2)
#define PPP_SRC_C2R_5BIT (1<<4)

#define PPP_SRC_C3ALPHA_EN (1<<8)

#define PPP_SRC_BPP_1BYTES 0
#define PPP_SRC_BPP_2BYTES (1<<9)
#define PPP_SRC_BPP_3BYTES (1<<10)
#define PPP_SRC_BPP_4BYTES ((1<<10)|(1<<9))

#define PPP_SRC_BPP_ROI_ODD_X (1<<11)
#define PPP_SRC_BPP_ROI_ODD_Y (1<<12)
#define PPP_SRC_INTERLVD_2COMPONENTS (1<<13)
#define PPP_SRC_INTERLVD_3COMPONENTS (1<<14)
#define PPP_SRC_INTERLVD_4COMPONENTS ((1<<14)|(1<<13))


/* RGB666 unpack format
** TIGHT means R6+G6+B6 together
** LOOSE means R6+2 +G6+2+ B6+2 (with MSB)
**          or 2+R6 +2+G6 +2+B6 (with LSB)
*/
#define PPP_SRC_PACK_TIGHT (1<<17)
#define PPP_SRC_PACK_LOOSE 0
#define PPP_SRC_PACK_ALIGN_LSB 0
#define PPP_SRC_PACK_ALIGN_MSB (1<<18)

#define PPP_SRC_PLANE_INTERLVD 0
#define PPP_SRC_PLANE_PSEUDOPLNR (1<<20)

#define PPP_SRC_WMV9_MODE (1<<21)

/* MDP_PPP_OPERATION_CONFIG / MDP_FULL_BYPASS_WORD14 */
#define PPP_OP_SCALE_X_ON (1<<0)
#define PPP_OP_SCALE_Y_ON (1<<1)

#ifndef CONFIG_MSM_MDP31
#define PPP_OP_CONVERT_RGB2YCBCR 0
#else
#define PPP_OP_CONVERT_RGB2YCBCR (1<<30)
#endif

#define PPP_OP_CONVERT_YCBCR2RGB (1<<2)
#define PPP_OP_CONVERT_ON (1<<3)

#define PPP_OP_CONVERT_MATRIX_PRIMARY 0
#define PPP_OP_CONVERT_MATRIX_SECONDARY (1<<4)

#define PPP_OP_LUT_C0_ON (1<<5)
#define PPP_OP_LUT_C1_ON (1<<6)
#define PPP_OP_LUT_C2_ON (1<<7)

/* rotate or blend enable */
#define PPP_OP_ROT_ON (1<<8)

#define PPP_OP_ROT_90 (1<<9)
#define PPP_OP_FLIP_LR (1<<10)
#define PPP_OP_FLIP_UD (1<<11)

#define PPP_OP_BLEND_ON (1<<12)

#define PPP_OP_BLEND_SRCPIXEL_ALPHA 0
#define PPP_OP_BLEND_DSTPIXEL_ALPHA (1<<13)
#define PPP_OP_BLEND_CONSTANT_ALPHA (1<<14)
#define PPP_OP_BLEND_SRCPIXEL_TRANSP ((1<<13)|(1<<14))

#define PPP_OP_BLEND_ALPHA_BLEND_NORMAL 0
#define PPP_OP_BLEND_ALPHA_BLEND_REVERSE (1<<15)

#define PPP_OP_DITHER_EN (1<<16)

#define PPP_OP_COLOR_SPACE_RGB 0
#define PPP_OP_COLOR_SPACE_YCBCR (1<<17)

#define PPP_OP_SRC_CHROMA_RGB 0
#define PPP_OP_SRC_CHROMA_H2V1 (1<<18)
#define PPP_OP_SRC_CHROMA_H1V2 (1<<19)
#define PPP_OP_SRC_CHROMA_420 ((1<<18)|(1<<19))
#define PPP_OP_SRC_CHROMA_COSITE 0
#define PPP_OP_SRC_CHROMA_OFFSITE (1<<20)

#define PPP_OP_DST_CHROMA_RGB 0
#define PPP_OP_DST_CHROMA_H2V1 (1<<21)
#define PPP_OP_DST_CHROMA_H1V2 (1<<22)
#define PPP_OP_DST_CHROMA_420 ((1<<21)|(1<<22))
#define PPP_OP_DST_CHROMA_COSITE 0
#define PPP_OP_DST_CHROMA_OFFSITE (1<<23)

#define PPP_BLEND_ALPHA_TRANSP (1<<24)

#define PPP_OP_BG_CHROMA_RGB 0
#define PPP_OP_BG_CHROMA_H2V1 (1<<25)
#define PPP_OP_BG_CHROMA_H1V2 (1<<26)
#define PPP_OP_BG_CHROMA_420 ((1<<25)|(1<<26))
#define PPP_OP_BG_CHROMA_SITE_COSITE 0
#define PPP_OP_BG_CHROMA_SITE_OFFSITE (1<<27)

#define PPP_BLEND_BG_USE_ALPHA_SEL      (1 << 0)
#define PPP_BLEND_BG_ALPHA_REVERSE      (1 << 3)
#define PPP_BLEND_BG_SRCPIXEL_ALPHA     (0 << 1)
#define PPP_BLEND_BG_DSTPIXEL_ALPHA     (1 << 1)
#define PPP_BLEND_BG_CONSTANT_ALPHA     (2 << 1)
#define PPP_BLEND_BG_CONST_ALPHA_VAL(x) ((x) << 24)

/* MDP_PPP_DESTINATION_CONFIG / MDP_FULL_BYPASS_WORD20 */
#define PPP_DST_C0G_8BIT ((1<<0)|(1<<1))
#define PPP_DST_C1B_8BIT ((1<<3)|(1<<2))
#define PPP_DST_C2R_8BIT ((1<<5)|(1<<4))
#define PPP_DST_C3A_8BIT ((1<<7)|(1<<6))

#define PPP_DST_C0G_6BIT (1<<1)
#define PPP_DST_C1B_6BIT (1<<3)
#define PPP_DST_C2R_6BIT (1<<5)

#define PPP_DST_C0G_5BIT (1<<0)
#define PPP_DST_C1B_5BIT (1<<2)
#define PPP_DST_C2R_5BIT (1<<4)

#define PPP_DST_C3A_8BIT ((1<<7)|(1<<6))
#define PPP_DST_C3ALPHA_EN (1<<8)

#define PPP_DST_INTERLVD_2COMPONENTS (1<<9)
#define PPP_DST_INTERLVD_3COMPONENTS (1<<10)
#define PPP_DST_INTERLVD_4COMPONENTS ((1<<10)|(1<<9))
#define PPP_DST_INTERLVD_6COMPONENTS ((1<<11)|(1<<9))

#define PPP_DST_PACK_LOOSE 0
#define PPP_DST_PACK_TIGHT (1<<13)
#define PPP_DST_PACK_ALIGN_LSB 0
#define PPP_DST_PACK_ALIGN_MSB (1<<14)

#define PPP_DST_OUT_SEL_AXI 0
#define PPP_DST_OUT_SEL_MDDI (1<<15)

#define PPP_DST_BPP_2BYTES (1<<16)
#define PPP_DST_BPP_3BYTES (1<<17)
#define PPP_DST_BPP_4BYTES ((1<<17)|(1<<16))

#define PPP_DST_PLANE_INTERLVD 0
#define PPP_DST_PLANE_PLANAR (1<<18)
#define PPP_DST_PLANE_PSEUDOPLNR (1<<19)

#define PPP_DST_TO_TV (1<<20)

#define PPP_DST_MDDI_PRIMARY 0
#define PPP_DST_MDDI_SECONDARY (1<<21)
#define PPP_DST_MDDI_EXTERNAL (1<<22)

/* image configurations by image type */
#define PPP_CFG_MDP_RGB_565(dir)       (PPP_##dir##_C2R_5BIT | \
					PPP_##dir##_C0G_6BIT | \
					PPP_##dir##_C1B_5BIT | \
					PPP_##dir##_BPP_2BYTES | \
					PPP_##dir##_INTERLVD_3COMPONENTS | \
					PPP_##dir##_PACK_TIGHT | \
					PPP_##dir##_PACK_ALIGN_LSB | \
					PPP_##dir##_PLANE_INTERLVD)

#define PPP_CFG_MDP_RGB_888(dir)       (PPP_##dir##_C2R_8BIT | \
					PPP_##dir##_C0G_8BIT | \
					PPP_##dir##_C1B_8BIT | \
					PPP_##dir##_BPP_3BYTES | \
					PPP_##dir##_INTERLVD_3COMPONENTS | \
					PPP_##dir##_PACK_TIGHT | \
					PPP_##dir##_PACK_ALIGN_LSB | \
					PPP_##dir##_PLANE_INTERLVD)

#define PPP_CFG_MDP_ARGB_8888(dir)     (PPP_##dir##_C2R_8BIT | \
					PPP_##dir##_C0G_8BIT | \
					PPP_##dir##_C1B_8BIT | \
					PPP_##dir##_C3A_8BIT | \
					PPP_##dir##_C3ALPHA_EN | \
					PPP_##dir##_BPP_4BYTES | \
					PPP_##dir##_INTERLVD_4COMPONENTS | \
					PPP_##dir##_PACK_TIGHT | \
					PPP_##dir##_PACK_ALIGN_LSB | \
					PPP_##dir##_PLANE_INTERLVD)

#define PPP_CFG_MDP_XRGB_8888(dir) PPP_CFG_MDP_ARGB_8888(dir)
#define PPP_CFG_MDP_RGBA_8888(dir) PPP_CFG_MDP_ARGB_8888(dir)
#define PPP_CFG_MDP_BGRA_8888(dir) PPP_CFG_MDP_ARGB_8888(dir)
#define PPP_CFG_MDP_RGBX_8888(dir) PPP_CFG_MDP_ARGB_8888(dir)

#define PPP_CFG_MDP_Y_CBCR_H2V2(dir)   (PPP_##dir##_C2R_8BIT | \
					PPP_##dir##_C0G_8BIT | \
					PPP_##dir##_C1B_8BIT | \
					PPP_##dir##_C3A_8BIT | \
					PPP_##dir##_BPP_2BYTES | \
					PPP_##dir##_INTERLVD_2COMPONENTS | \
					PPP_##dir##_PACK_TIGHT | \
					PPP_##dir##_PACK_ALIGN_LSB | \
					PPP_##dir##_PLANE_PSEUDOPLNR)

#define PPP_CFG_MDP_Y_CRCB_H2V2(dir)	PPP_CFG_MDP_Y_CBCR_H2V2(dir)

#define PPP_CFG_MDP_YCRYCB_H2V1(dir)   (PPP_##dir##_C2R_8BIT | \
					PPP_##dir##_C0G_8BIT | \
					PPP_##dir##_C1B_8BIT | \
					PPP_##dir##_C3A_8BIT | \
					PPP_##dir##_BPP_2BYTES | \
					PPP_##dir##_INTERLVD_4COMPONENTS | \
					PPP_##dir##_PACK_TIGHT | \
					PPP_##dir##_PACK_ALIGN_LSB |\
					PPP_##dir##_PLANE_INTERLVD)

#define PPP_CFG_MDP_Y_CBCR_H2V1(dir)   (PPP_##dir##_C2R_8BIT | \
					PPP_##dir##_C0G_8BIT | \
					PPP_##dir##_C1B_8BIT | \
					PPP_##dir##_C3A_8BIT | \
					PPP_##dir##_BPP_2BYTES |   \
					PPP_##dir##_INTERLVD_2COMPONENTS |  \
					PPP_##dir##_PACK_TIGHT | \
					PPP_##dir##_PACK_ALIGN_LSB | \
					PPP_##dir##_PLANE_PSEUDOPLNR)

#define PPP_CFG_MDP_Y_CRCB_H2V1(dir)	PPP_CFG_MDP_Y_CBCR_H2V1(dir)

#define PPP_PACK_PATTERN_MDP_RGB_565 \
	MDP_GET_PACK_PATTERN(0, CLR_R, CLR_G, CLR_B, 8)
#define PPP_PACK_PATTERN_MDP_RGB_888 PPP_PACK_PATTERN_MDP_RGB_565
#define PPP_PACK_PATTERN_MDP_XRGB_8888 \
	MDP_GET_PACK_PATTERN(CLR_B, CLR_G, CLR_R, CLR_ALPHA, 8)
#define PPP_PACK_PATTERN_MDP_ARGB_8888 PPP_PACK_PATTERN_MDP_XRGB_8888
#define PPP_PACK_PATTERN_MDP_RGBA_8888 \
	MDP_GET_PACK_PATTERN(CLR_ALPHA, CLR_B, CLR_G, CLR_R, 8)
#define PPP_PACK_PATTERN_MDP_BGRA_8888 \
	MDP_GET_PACK_PATTERN(CLR_ALPHA, CLR_R, CLR_G, CLR_B, 8)
#define PPP_PACK_PATTERN_MDP_RGBX_8888 \
	MDP_GET_PACK_PATTERN(CLR_ALPHA, CLR_B, CLR_G, CLR_R, 8)
#define PPP_PACK_PATTERN_MDP_Y_CBCR_H2V1 \
	MDP_GET_PACK_PATTERN(0, 0, CLR_CB, CLR_CR, 8)
#define PPP_PACK_PATTERN_MDP_Y_CBCR_H2V2 PPP_PACK_PATTERN_MDP_Y_CBCR_H2V1
#define PPP_PACK_PATTERN_MDP_Y_CRCB_H2V1 \
	MDP_GET_PACK_PATTERN(0, 0, CLR_CR, CLR_CB, 8)
#define PPP_PACK_PATTERN_MDP_Y_CRCB_H2V2 PPP_PACK_PATTERN_MDP_Y_CRCB_H2V1
#define PPP_PACK_PATTERN_MDP_YCRYCB_H2V1 \
	MDP_GET_PACK_PATTERN(CLR_Y, CLR_R, CLR_Y, CLR_B, 8)

#define PPP_CHROMA_SAMP_MDP_RGB_565(dir) PPP_OP_##dir##_CHROMA_RGB
#define PPP_CHROMA_SAMP_MDP_RGB_888(dir) PPP_OP_##dir##_CHROMA_RGB
#define PPP_CHROMA_SAMP_MDP_XRGB_8888(dir) PPP_OP_##dir##_CHROMA_RGB
#define PPP_CHROMA_SAMP_MDP_ARGB_8888(dir) PPP_OP_##dir##_CHROMA_RGB
#define PPP_CHROMA_SAMP_MDP_RGBA_8888(dir) PPP_OP_##dir##_CHROMA_RGB
#define PPP_CHROMA_SAMP_MDP_BGRA_8888(dir) PPP_OP_##dir##_CHROMA_RGB
#define PPP_CHROMA_SAMP_MDP_RGBX_8888(dir) PPP_OP_##dir##_CHROMA_RGB
#define PPP_CHROMA_SAMP_MDP_Y_CBCR_H2V1(dir) PPP_OP_##dir##_CHROMA_H2V1
#define PPP_CHROMA_SAMP_MDP_Y_CBCR_H2V2(dir) PPP_OP_##dir##_CHROMA_420
#define PPP_CHROMA_SAMP_MDP_Y_CRCB_H2V1(dir) PPP_OP_##dir##_CHROMA_H2V1
#define PPP_CHROMA_SAMP_MDP_Y_CRCB_H2V2(dir) PPP_OP_##dir##_CHROMA_420
#define PPP_CHROMA_SAMP_MDP_YCRYCB_H2V1(dir) PPP_OP_##dir##_CHROMA_H2V1

/* Helpful array generation macros */
#define PPP_ARRAY0(name) \
	[MDP_RGB_565] = PPP_##name##_MDP_RGB_565,\
	[MDP_RGB_888] = PPP_##name##_MDP_RGB_888,\
	[MDP_XRGB_8888] = PPP_##name##_MDP_XRGB_8888,\
	[MDP_ARGB_8888] = PPP_##name##_MDP_ARGB_8888,\
	[MDP_RGBA_8888] = PPP_##name##_MDP_RGBA_8888,\
	[MDP_BGRA_8888] = PPP_##name##_MDP_BGRA_8888,\
	[MDP_RGBX_8888] = PPP_##name##_MDP_RGBX_8888,\
	[MDP_Y_CBCR_H2V1] = PPP_##name##_MDP_Y_CBCR_H2V1,\
	[MDP_Y_CBCR_H2V2] = PPP_##name##_MDP_Y_CBCR_H2V2,\
	[MDP_Y_CRCB_H2V1] = PPP_##name##_MDP_Y_CRCB_H2V1,\
	[MDP_Y_CRCB_H2V2] = PPP_##name##_MDP_Y_CRCB_H2V2,\
	[MDP_YCRYCB_H2V1] = PPP_##name##_MDP_YCRYCB_H2V1

#define PPP_ARRAY1(name, dir) \
	[MDP_RGB_565] = PPP_##name##_MDP_RGB_565(dir),\
	[MDP_RGB_888] = PPP_##name##_MDP_RGB_888(dir),\
	[MDP_XRGB_8888] = PPP_##name##_MDP_XRGB_8888(dir),\
	[MDP_ARGB_8888] = PPP_##name##_MDP_ARGB_8888(dir),\
	[MDP_RGBA_8888] = PPP_##name##_MDP_RGBA_8888(dir),\
	[MDP_BGRA_8888] = PPP_##name##_MDP_BGRA_8888(dir),\
	[MDP_RGBX_8888] = PPP_##name##_MDP_RGBX_8888(dir),\
	[MDP_Y_CBCR_H2V1] = PPP_##name##_MDP_Y_CBCR_H2V1(dir),\
	[MDP_Y_CBCR_H2V2] = PPP_##name##_MDP_Y_CBCR_H2V2(dir),\
	[MDP_Y_CRCB_H2V1] = PPP_##name##_MDP_Y_CRCB_H2V1(dir),\
	[MDP_Y_CRCB_H2V2] = PPP_##name##_MDP_Y_CRCB_H2V2(dir),\
	[MDP_YCRYCB_H2V1] = PPP_##name##_MDP_YCRYCB_H2V1(dir)

#define IS_YCRCB(img) ((img == MDP_Y_CRCB_H2V2) | (img == MDP_Y_CBCR_H2V2) | \
		       (img == MDP_Y_CRCB_H2V1) | (img == MDP_Y_CBCR_H2V1) | \
		       (img == MDP_YCRYCB_H2V1))
#define IS_RGB(img) ((img == MDP_RGB_565) | (img == MDP_RGB_888) | \
		     (img == MDP_ARGB_8888) | (img == MDP_RGBA_8888) | \
		     (img == MDP_XRGB_8888) | (img == MDP_BGRA_8888) | \
		     (img == MDP_RGBX_8888))
#define HAS_ALPHA(img) ((img == MDP_ARGB_8888) | (img == MDP_RGBA_8888) | \
			(img == MDP_BGRA_8888))

#define IS_PSEUDOPLNR(img) ((img == MDP_Y_CRCB_H2V2) | \
			    (img == MDP_Y_CBCR_H2V2) | \
			    (img == MDP_Y_CRCB_H2V1) | \
			    (img == MDP_Y_CBCR_H2V1))

/* Mappings from addr to purpose */
#define PPP_ADDR_SRC_ROI		MDP_FULL_BYPASS_WORD2
#define PPP_ADDR_SRC0			MDP_FULL_BYPASS_WORD3
#define PPP_ADDR_SRC1			MDP_FULL_BYPASS_WORD4
#define PPP_ADDR_SRC_YSTRIDE		MDP_FULL_BYPASS_WORD7
#define PPP_ADDR_SRC_CFG		MDP_FULL_BYPASS_WORD9
#define PPP_ADDR_SRC_PACK_PATTERN	MDP_FULL_BYPASS_WORD10
#define PPP_ADDR_OPERATION		MDP_FULL_BYPASS_WORD14
#define PPP_ADDR_PHASEX_INIT		MDP_FULL_BYPASS_WORD15
#define PPP_ADDR_PHASEY_INIT		MDP_FULL_BYPASS_WORD16
#define PPP_ADDR_PHASEX_STEP		MDP_FULL_BYPASS_WORD17
#define PPP_ADDR_PHASEY_STEP		MDP_FULL_BYPASS_WORD18
#define PPP_ADDR_ALPHA_TRANSP		MDP_FULL_BYPASS_WORD19
#define PPP_ADDR_DST_CFG		MDP_FULL_BYPASS_WORD20
#define PPP_ADDR_DST_PACK_PATTERN	MDP_FULL_BYPASS_WORD21
#define PPP_ADDR_DST_ROI		MDP_FULL_BYPASS_WORD25
#define PPP_ADDR_DST0			MDP_FULL_BYPASS_WORD26
#define PPP_ADDR_DST1			MDP_FULL_BYPASS_WORD27
#define PPP_ADDR_DST_YSTRIDE		MDP_FULL_BYPASS_WORD30
#define PPP_ADDR_EDGE			MDP_FULL_BYPASS_WORD46
#define PPP_ADDR_BG0			MDP_FULL_BYPASS_WORD48
#define PPP_ADDR_BG1			MDP_FULL_BYPASS_WORD49
#define PPP_ADDR_BG_YSTRIDE		MDP_FULL_BYPASS_WORD51
#define PPP_ADDR_BG_CFG			MDP_FULL_BYPASS_WORD53
#define PPP_ADDR_BG_PACK_PATTERN	MDP_FULL_BYPASS_WORD54

/* color conversion matrix configuration registers */
/* pfmv is mv1, prmv is mv2 */
#define MDP_CSC_PFMVn(n)		(0x40400 + (4 * (n)))
#define MDP_CSC_PRMVn(n)		(0x40440 + (4 * (n)))

#ifdef CONFIG_MSM_MDP31
#define MDP_PPP_CSC_PRE_BV1n(n)		(0x40500 + (4 * (n)))
#define MDP_PPP_CSC_PRE_BV2n(n)		(0x40540 + (4 * (n)))
#define MDP_PPP_CSC_POST_BV1n(n)	(0x40580 + (4 * (n)))
#define MDP_PPP_CSC_POST_BV2n(n)	(0x405c0 + (4 * (n)))

#define MDP_PPP_CSC_PRE_LV1n(n)		(0x40600 + (4 * (n)))
#define MDP_PPP_CSC_PRE_LV2n(n)		(0x40640 + (4 * (n)))
#define MDP_PPP_CSC_POST_LV1n(n)	(0x40680 + (4 * (n)))
#define MDP_PPP_CSC_POST_LV2n(n)	(0x406c0 + (4 * (n)))

#define MDP_PPP_SCALE_COEFF_D0_SET	(0)
#define MDP_PPP_SCALE_COEFF_D1_SET	(1)
#define MDP_PPP_SCALE_COEFF_D2_SET	(2)
#define MDP_PPP_SCALE_COEFF_U1_SET	(3)
#define MDP_PPP_SCALE_COEFF_LSBn(n)	(0x50400 + (8 * (n)))
#define MDP_PPP_SCALE_COEFF_MSBn(n)	(0x50404 + (8 * (n)))

#define MDP_PPP_DEINT_COEFFn(n)		(0x30010 + (4 * (n)))

#define MDP_PPP_SCALER_FIR		(0)
#define MDP_PPP_SCALER_MN		(1)

#else /* !defined(CONFIG_MSM_MDP31) */

#define MDP_CSC_PBVn(n)			(0x40500 + (4 * (n)))
#define MDP_CSC_SBVn(n)			(0x40540 + (4 * (n)))
#define MDP_CSC_PLVn(n)			(0x40580 + (4 * (n)))
#define MDP_CSC_SLVn(n)			(0x405c0 + (4 * (n)))

#endif


/* MDP_DMA_CONFIG / MDP_FULL_BYPASS_WORD32 */
#define DMA_DSTC0G_5BITS (1<<0)
#define DMA_DSTC1B_5BITS (1<<2)
#define DMA_DSTC2R_5BITS (1<<4)

#define DMA_DSTC0G_6BITS (2<<0)
#define DMA_DSTC1B_6BITS (2<<2)
#define DMA_DSTC2R_6BITS (2<<4)

#define DMA_DSTC0G_8BITS (3<<0)
#define DMA_DSTC1B_8BITS (3<<2)
#define DMA_DSTC2R_8BITS (3<<4)

#define DMA_DST_BITS_MASK 0x3F

#define DMA_PACK_TIGHT (1<<6)
#define DMA_PACK_LOOSE 0
#define DMA_PACK_ALIGN_LSB 0
#define DMA_PACK_ALIGN_MSB (1<<7)
#define DMA_PACK_PATTERN_MASK (0x3f<<8)
#define DMA_PACK_PATTERN_RGB \
	(MDP_GET_PACK_PATTERN(0, CLR_R, CLR_G, CLR_B, 2)<<8)
#define DMA_PACK_PATTERN_BGR \
	(MDP_GET_PACK_PATTERN(0, CLR_B, CLR_G, CLR_R, 2)<<8)


#ifdef CONFIG_MSM_MDP22

#define DMA_OUT_SEL_AHB  0
#define DMA_OUT_SEL_MDDI (1<<14)
#define DMA_AHBM_LCD_SEL_PRIMARY 0
#define DMA_AHBM_LCD_SEL_SECONDARY (1<<15)
#define DMA_IBUF_C3ALPHA_EN (1<<16)
#define DMA_DITHER_EN (1<<17)
#define DMA_MDDI_DMAOUT_LCD_SEL_PRIMARY 0
#define DMA_MDDI_DMAOUT_LCD_SEL_SECONDARY (1<<18)
#define DMA_MDDI_DMAOUT_LCD_SEL_EXTERNAL (1<<19)
#define DMA_IBUF_FORMAT_RGB565 (1<<20)
#define DMA_IBUF_FORMAT_RGB888_OR_ARGB8888 0
#define DMA_IBUF_FORMAT_MASK (1 << 20)
#define DMA_IBUF_NONCONTIGUOUS (1<<21)

#elif defined(CONFIG_MSM_MDP30)

#define DMA_OUT_SEL_AHB  0
#define DMA_OUT_SEL_MDDI (1<<19)
#define DMA_AHBM_LCD_SEL_PRIMARY 0
#define DMA_AHBM_LCD_SEL_SECONDARY (0)
#define DMA_IBUF_C3ALPHA_EN (0)
#define DMA_DITHER_EN (1<<24)

#define DMA_MDDI_DMAOUT_LCD_SEL_PRIMARY 0
#define DMA_MDDI_DMAOUT_LCD_SEL_SECONDARY (0)
#define DMA_MDDI_DMAOUT_LCD_SEL_EXTERNAL (0)

#define DMA_IBUF_FORMAT_MASK (1 << 20)
#define DMA_IBUF_FORMAT_RGB565 (1<<25)
#define DMA_IBUF_FORMAT_RGB888_OR_ARGB8888 (1<<26)
#define DMA_IBUF_NONCONTIGUOUS (0)

#else /* CONFIG_MSM_MDP31 | CONFIG_MSM_MDP302 */

#define DMA_OUT_SEL_AHB				(0 << 19)
#define DMA_OUT_SEL_MDDI			(1 << 19)
#define DMA_OUT_SEL_LCDC			(2 << 19)
#define DMA_OUT_SEL_LCDC_MDDI			(3 << 19)
#define DMA_DITHER_EN				(1 << 24)
#define DMA_IBUF_FORMAT_RGB888			(0 << 25)
#define DMA_IBUF_FORMAT_RGB565			(1 << 25)
#define DMA_IBUF_FORMAT_XRGB8888		(2 << 25)
#define DMA_IBUF_FORMAT_MASK			(3 << 25)
#define DMA_IBUF_NONCONTIGUOUS			(0)

#define DMA_MDDI_DMAOUT_LCD_SEL_PRIMARY		(0)
#define DMA_MDDI_DMAOUT_LCD_SEL_SECONDARY	(0)
#define DMA_MDDI_DMAOUT_LCD_SEL_EXTERNAL	(0)
#endif

/* MDDI REGISTER ? */
#define MDDI_VDO_PACKET_DESC_RGB565  0x5565
#define MDDI_VDO_PACKET_DESC_RGB666  0x5666
#define MDDI_VDO_PACKET_PRIM  0xC3
#define MDDI_VDO_PACKET_SECD  0xC0

#endif
