// Copyright 2015-2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>
#include "soc/soc.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/apb_ctrl_reg.h"
#include "soc/dport_reg.h"
#include "soc/rtc.h"
#include "soc/i2s_reg.h"
#include "soc/timer_group_reg.h"
#include "soc/bb_reg.h"
#include "soc/nrx_reg.h"
#include "soc/fe_reg.h"
#include "soc/rtc.h"
#include "regi2c_ctrl.h"

/**
 * Configure whether certain peripherals are powered up in sleep
 * @param cfg power down flags as rtc_sleep_pu_config_t structure
 */
void rtc_sleep_pu(rtc_sleep_pu_config_t cfg)
{
    REG_SET_FIELD(RTC_CNTL_DIG_PWC_REG, RTC_CNTL_LSLP_MEM_FORCE_PU, cfg.dig_fpu);
    REG_SET_FIELD(RTC_CNTL_PWC_REG, RTC_CNTL_FASTMEM_FORCE_LPU, cfg.rtc_fpu);
    REG_SET_FIELD(RTC_CNTL_PWC_REG, RTC_CNTL_SLOWMEM_FORCE_LPU, cfg.rtc_fpu);
    REG_SET_FIELD(APB_CTRL_FRONT_END_MEM_PD_REG, APB_CTRL_DC_MEM_FORCE_PU, cfg.fe_fpu);
    REG_SET_FIELD(APB_CTRL_FRONT_END_MEM_PD_REG, APB_CTRL_PBUS_MEM_FORCE_PU, cfg.fe_fpu);
    REG_SET_FIELD(APB_CTRL_FRONT_END_MEM_PD_REG, APB_CTRL_AGC_MEM_FORCE_PU, cfg.fe_fpu);
    REG_SET_FIELD(BBPD_CTRL, BB_FFT_FORCE_PU, cfg.bb_fpu);
    REG_SET_FIELD(BBPD_CTRL, BB_DC_EST_FORCE_PU, cfg.bb_fpu);
    REG_SET_FIELD(NRXPD_CTRL, NRX_RX_ROT_FORCE_PU, cfg.nrx_fpu);
    REG_SET_FIELD(NRXPD_CTRL, NRX_VIT_FORCE_PU, cfg.nrx_fpu);
    REG_SET_FIELD(NRXPD_CTRL, NRX_DEMAP_FORCE_PU, cfg.nrx_fpu);
    REG_SET_FIELD(FE_GEN_CTRL, FE_IQ_EST_FORCE_PU, cfg.fe_fpu);
    REG_SET_FIELD(FE2_TX_INTERP_CTRL, FE2_TX_INF_FORCE_PU, cfg.fe_fpu);
    if (cfg.sram_fpu) {
        REG_SET_FIELD(SYSTEM_SRAM_CTRL_2_REG, SYSTEM_SRAM_POWER_UP, SYSTEM_SRAM_POWER_UP);
    } else {
        REG_SET_FIELD(SYSTEM_SRAM_CTRL_2_REG, SYSTEM_SRAM_POWER_UP, 0);
    }
    if (cfg.rom_ram_fpu) {
        SET_PERI_REG_MASK(SYSTEM_ROM_CTRL_1_REG, SYSTEM_ROM_IRAM0_DRAM0_POWER_UP);
        REG_SET_FIELD(SYSTEM_ROM_CTRL_1_REG, SYSTEM_ROM_IRAM0_POWER_UP, SYSTEM_ROM_IRAM0_POWER_UP);
    } else {
        CLEAR_PERI_REG_MASK(SYSTEM_ROM_CTRL_1_REG, SYSTEM_ROM_IRAM0_DRAM0_POWER_UP);
        REG_SET_FIELD(SYSTEM_ROM_CTRL_1_REG, SYSTEM_ROM_IRAM0_POWER_UP, 0);
    }
}

void rtc_sleep_get_default_config(uint32_t sleep_flags, rtc_sleep_config_t *out_config)
{
    *out_config = (rtc_sleep_config_t) {
        .lslp_mem_inf_fpu = 0,
        .rtc_mem_inf_follow_cpu = (sleep_flags & RTC_SLEEP_PD_RTC_MEM_FOLLOW_CPU) ? 1 : 0,
        .rtc_fastmem_pd_en = (sleep_flags & RTC_SLEEP_PD_RTC_FAST_MEM) ? 1 : 0,
        .rtc_slowmem_pd_en = (sleep_flags & RTC_SLEEP_PD_RTC_SLOW_MEM) ? 1 : 0,
        .rtc_peri_pd_en = (sleep_flags & RTC_SLEEP_PD_RTC_PERIPH) ? 1 : 0,
        .wifi_pd_en = (sleep_flags & RTC_SLEEP_PD_WIFI) ? 1 : 0,
        .bt_pd_en = (sleep_flags & RTC_SLEEP_PD_BT) ? 1 : 0,
        .cpu_pd_en = (sleep_flags & RTC_SLEEP_PD_CPU) ? 1 : 0,
        .int_8m_pd_en = (sleep_flags & RTC_SLEEP_PD_INT_8M) ? 1 : 0,
        .dig_peri_pd_en = (sleep_flags & RTC_SLEEP_PD_DIG_PERIPH) ? 1 : 0,
        .deep_slp = (sleep_flags & RTC_SLEEP_PD_DIG) ? 1 : 0,
        .wdt_flashboot_mod_en = 0,
        .vddsdio_pd_en = (sleep_flags & RTC_SLEEP_PD_VDDSDIO) ? 1 : 0,
        .xtal_fpu = (sleep_flags & RTC_SLEEP_PD_XTAL) ? 0 : 1,
        .deep_slp_reject = 1,
        .light_slp_reject = 1
    };

    if (sleep_flags & RTC_SLEEP_PD_DIG) {
        out_config->dig_dbias_wak = RTC_CNTL_DBIAS_1V10;
        out_config->dig_dbias_slp = RTC_CNTL_DBIAS_SLP;
        out_config->rtc_dbias_wak = RTC_CNTL_DBIAS_1V10;
        out_config->rtc_dbias_slp = RTC_CNTL_DBIAS_SLP;

        out_config->dbg_atten_monitor = RTC_CNTL_DBG_ATTEN_MONITOR_DEFAULT;
        out_config->bias_sleep_monitor = RTC_CNTL_BIASSLP_MONITOR_DEFAULT;
        out_config->dbg_atten_slp = RTC_CNTL_DBG_ATTEN_DEEPSLEEP_DEFAULT;
        out_config->bias_sleep_slp = RTC_CNTL_BIASSLP_SLEEP_DEFAULT;
        out_config->pd_cur_monitor = RTC_CNTL_PD_CUR_MONITOR_DEFAULT;
        out_config->pd_cur_slp = RTC_CNTL_PD_CUR_SLEEP_DEFAULT;
    } else {
        out_config->dig_dbias_wak = RTC_CNTL_DBIAS_1V10;
        out_config->dig_dbias_slp = !(sleep_flags & RTC_SLEEP_PD_INT_8M) ? RTC_CNTL_DBIAS_1V10 : RTC_CNTL_DBIAS_SLP;
        out_config->rtc_dbias_wak = RTC_CNTL_DBIAS_1V10;
        out_config->rtc_dbias_slp = !(sleep_flags & RTC_SLEEP_PD_INT_8M) ? RTC_CNTL_DBIAS_1V10 : RTC_CNTL_DBIAS_SLP;

        out_config->dbg_atten_monitor = RTC_CNTL_DBG_ATTEN_MONITOR_DEFAULT;
        out_config->bias_sleep_monitor = RTC_CNTL_BIASSLP_MONITOR_DEFAULT;
        out_config->dbg_atten_slp = (sleep_flags & RTC_SLEEP_PD_INT_8M) ? RTC_CNTL_DBG_ATTEN_LIGHTSLEEP_DEFAULT : RTC_CNTL_DBG_ATTEN_LIGHTSLEEP_NODROP;
        out_config->bias_sleep_slp = !(sleep_flags & RTC_SLEEP_PD_XTAL) ? RTC_CNTL_BIASSLP_SLEEP_ON : RTC_CNTL_BIASSLP_SLEEP_DEFAULT;
        out_config->pd_cur_monitor = RTC_CNTL_PD_CUR_MONITOR_DEFAULT;
        out_config->pd_cur_slp = !(sleep_flags & RTC_SLEEP_PD_XTAL) ? RTC_CNTL_PD_CUR_SLEEP_ON : RTC_CNTL_PD_CUR_SLEEP_DEFAULT;
    }
}

void rtc_sleep_init(rtc_sleep_config_t cfg)
{
    if (cfg.lslp_mem_inf_fpu) {
        rtc_sleep_pu_config_t pu_cfg = RTC_SLEEP_PU_CONFIG_ALL(1);
        rtc_sleep_pu(pu_cfg);
    }

    if (cfg.rtc_mem_inf_follow_cpu) {
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_MEM_FOLW_CPU);
    } else {
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_MEM_FOLW_CPU);
    }

    if (cfg.rtc_fastmem_pd_en) {
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_FASTMEM_PD_EN);
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_FASTMEM_FORCE_PU);
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_FASTMEM_FORCE_NOISO);
    } else {
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_FASTMEM_PD_EN);
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_FASTMEM_FORCE_PU);
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_FASTMEM_FORCE_NOISO);
    }

    if (cfg.rtc_slowmem_pd_en) {
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_SLOWMEM_PD_EN);
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_SLOWMEM_FORCE_PU);
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_SLOWMEM_FORCE_NOISO);
    } else {
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_SLOWMEM_PD_EN);
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_SLOWMEM_FORCE_PU);
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_SLOWMEM_FORCE_NOISO);
    }

    if (cfg.rtc_peri_pd_en) {
        SET_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_PD_EN);
    } else {
        CLEAR_PERI_REG_MASK(RTC_CNTL_PWC_REG, RTC_CNTL_PD_EN);
    }

    if (cfg.wifi_pd_en) {
        SET_PERI_REG_MASK(RTC_CNTL_DIG_PWC_REG, RTC_CNTL_WIFI_PD_EN);
    } else {
        CLEAR_PERI_REG_MASK(RTC_CNTL_DIG_PWC_REG, RTC_CNTL_WIFI_PD_EN);
    }

    REGI2C_WRITE_MASK(I2C_DIG_REG, I2C_DIG_REG_EXT_RTC_DREG_SLEEP, cfg.rtc_dbias_slp);
    REGI2C_WRITE_MASK(I2C_DIG_REG, I2C_DIG_REG_EXT_RTC_DREG, cfg.rtc_dbias_wak);
    REGI2C_WRITE_MASK(I2C_DIG_REG, I2C_DIG_REG_EXT_DIG_DREG_SLEEP, cfg.dig_dbias_slp);
    REGI2C_WRITE_MASK(I2C_DIG_REG, I2C_DIG_REG_EXT_DIG_DREG, cfg.dig_dbias_wak);

    REG_SET_FIELD(RTC_CNTL_BIAS_CONF_REG, RTC_CNTL_DBG_ATTEN_MONITOR, cfg.dbg_atten_monitor);
    REG_SET_FIELD(RTC_CNTL_BIAS_CONF_REG, RTC_CNTL_BIAS_SLEEP_MONITOR, cfg.bias_sleep_monitor);
    REG_SET_FIELD(RTC_CNTL_BIAS_CONF_REG, RTC_CNTL_DBG_ATTEN_DEEP_SLP, cfg.dbg_atten_slp);
    REG_SET_FIELD(RTC_CNTL_BIAS_CONF_REG, RTC_CNTL_BIAS_SLEEP_DEEP_SLP, cfg.bias_sleep_slp);
    REG_SET_FIELD(RTC_CNTL_BIAS_CONF_REG, RTC_CNTL_PD_CUR_MONITOR, cfg.pd_cur_monitor);
    REG_SET_FIELD(RTC_CNTL_BIAS_CONF_REG, RTC_CNTL_PD_CUR_DEEP_SLP, cfg.pd_cur_slp);

    if (cfg.deep_slp) {
        CLEAR_PERI_REG_MASK(RTC_CNTL_REG, RTC_CNTL_REGULATOR_FORCE_PU);
        SET_PERI_REG_MASK(RTC_CNTL_DIG_PWC_REG, RTC_CNTL_DG_WRAP_PD_EN);
        CLEAR_PERI_REG_MASK(RTC_CNTL_ANA_CONF_REG,
                            RTC_CNTL_CKGEN_I2C_PU | RTC_CNTL_PLL_I2C_PU |
                            RTC_CNTL_RFRX_PBUS_PU | RTC_CNTL_TXRF_I2C_PU);
    } else {
        SET_PERI_REG_MASK(RTC_CNTL_REG, RTC_CNTL_REGULATOR_FORCE_PU);
        CLEAR_PERI_REG_MASK(RTC_CNTL_DIG_PWC_REG, RTC_CNTL_DG_WRAP_PD_EN);
    }

    /* enable VDDSDIO control by state machine */
    REG_CLR_BIT(RTC_CNTL_SDIO_CONF_REG, RTC_CNTL_SDIO_FORCE);
    REG_SET_FIELD(RTC_CNTL_SDIO_CONF_REG, RTC_CNTL_SDIO_PD_EN, cfg.vddsdio_pd_en);

    REG_SET_FIELD(RTC_CNTL_SLP_REJECT_CONF_REG, RTC_CNTL_DEEP_SLP_REJECT_EN, cfg.deep_slp_reject);
    REG_SET_FIELD(RTC_CNTL_SLP_REJECT_CONF_REG, RTC_CNTL_LIGHT_SLP_REJECT_EN, cfg.light_slp_reject);
}

void rtc_sleep_low_init(uint32_t slowclk_period)
{
    // set 5 PWC state machine times to fit in main state machine time
    REG_SET_FIELD(RTC_CNTL_TIMER1_REG, RTC_CNTL_PLL_BUF_WAIT, RTC_CNTL_PLL_BUF_WAIT_SLP_CYCLES);
    REG_SET_FIELD(RTC_CNTL_TIMER1_REG, RTC_CNTL_XTL_BUF_WAIT, rtc_time_us_to_slowclk(RTC_CNTL_XTL_BUF_WAIT_SLP_US, slowclk_period));
    REG_SET_FIELD(RTC_CNTL_TIMER1_REG, RTC_CNTL_CK8M_WAIT, RTC_CNTL_CK8M_WAIT_SLP_CYCLES);
}

void rtc_sleep_set_wakeup_time(uint64_t t)
{
    WRITE_PERI_REG(RTC_CNTL_SLP_TIMER0_REG, t & UINT32_MAX);
    WRITE_PERI_REG(RTC_CNTL_SLP_TIMER1_REG, t >> 32);
}

__attribute__((weak)) uint32_t rtc_sleep_start(uint32_t wakeup_opt, uint32_t reject_opt, uint32_t lslp_mem_inf_fpu)
{
    REG_SET_FIELD(RTC_CNTL_WAKEUP_STATE_REG, RTC_CNTL_WAKEUP_ENA, wakeup_opt);
    REG_SET_FIELD(RTC_CNTL_SLP_REJECT_CONF_REG, RTC_CNTL_SLEEP_REJECT_ENA, reject_opt);

    SET_PERI_REG_MASK(RTC_CNTL_INT_CLR_REG,
                      RTC_CNTL_SLP_REJECT_INT_CLR | RTC_CNTL_SLP_WAKEUP_INT_CLR);

    /* Start entry into sleep mode */
    SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);

    while (GET_PERI_REG_MASK(RTC_CNTL_INT_RAW_REG,
                             RTC_CNTL_SLP_REJECT_INT_RAW | RTC_CNTL_SLP_WAKEUP_INT_RAW) == 0) {
        ;
    }
    /* In deep sleep mode, we never get here */
    uint32_t reject = REG_GET_FIELD(RTC_CNTL_INT_RAW_REG, RTC_CNTL_SLP_REJECT_INT_RAW);
    SET_PERI_REG_MASK(RTC_CNTL_INT_CLR_REG,
                      RTC_CNTL_SLP_REJECT_INT_CLR | RTC_CNTL_SLP_WAKEUP_INT_CLR);

    /* restore config if it is a light sleep */
    if (lslp_mem_inf_fpu) {
        rtc_sleep_pu_config_t pu_cfg = RTC_SLEEP_PU_CONFIG_ALL(1);
        rtc_sleep_pu(pu_cfg);
    }
    return reject;
}
