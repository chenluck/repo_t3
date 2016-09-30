/*
 * hello_clocks.c              Copyright NXP 2016
 * Description: Example clock and LPIT channel initializations
 * 2016 Mar 04 S Mihalik - Initial version
 *
 */

#include "derivative.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h" /* include peripheral declarations S32K144 */

int lpit0_ch0_flag_counter = 0; /* LPIT0 timeout counter */

void init_ports (void) {
  PCC-> PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT D */
  PTD->PDDR |= 1<<0;            /* Port D0:  Data Direction= output */
  PORTD->PCR[0] =  0x00000100;  /* Port D0:  MUX = ALT1, GPIO (to blue LED on EVB) */
}
void init_LPIT0 (void) {
  PCC->PCCn[PCC_LPIT0_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
  PCC->PCCn[PCC_LPIT0_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs */
  LPIT0->MCR = 0x00000001;    /* DBG_EN-0: Timer chans stop in Debug mode */
                              /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                              /* SW_RST=0: SW reset does not reset timer chans, regs */
                              /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
  LPIT0->TVAL0 = 80000000;    /* Chan 0 Timeout period: 80M clocks */
  LPIT0->TCTRL0 = 0x00000001; /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}
void disable_WDOG (void){
	WDOG->CNT=0xD928C520; 	/*Unlock watchdog*/
	WDOG->TOVAL=0x0000FFFF;	/*Maximum timeout value*/
	WDOG->CS = 0x00002100;  /*Disable watchdog*/
}
int main(void) {
  disable_WDOG();
  init_ports();                  /* Configure ports */
  init_SOSC_8MHz();              /* Initialize system oscilator for 8 MHz xtal */
  init_SPLL_80MHz();             /* Initialize SPLL to 80 MHz with 8 MHz SOSC */
  SCG->RCCR=SCG_RCCR_SCS(6)  /* PLL as clock source*/
    |SCG_RCCR_DIVCORE(0b00)  /* DIVCORE= 1, Core clock = 80 MHz*/
    |SCG_RCCR_DIVBUS(0b01)   /* DIVBUS = 2, bus clock = 40 MHz*/
    |SCG_RCCR_DIVSLOW(0b10); /* DIVSLOW = 4, SCG slow, flash clock= 20 MHz*/
  if ((SCG->CSR & SCG_CSR_SCS_MASK >> SCG_CSR_SCS_SHIFT ) != 6) {} /* wait for sys clk src = SPLL */
  init_LPIT0();                  /* Initialize PIT0 for 1 second timeout  */
  for (;;) {                     /* Toggle output to LED every LPIT0 timeout */
    while (0 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK)) {} /* Wait for LPIT0 CH0 Flag */
    lpit0_ch0_flag_counter++;         /* Increment LPIT0 timeout counter */
    PTD->PTOR |= 1<<0;                /* Toggle output on port D0 (blue LED) */
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
  }
}
