#if 0
	// add these always since its not clear if we need to jump to main or main+2
    asm("nop");
    asm("nop");

	// March 24 204
	// 7800 needs to select on A12, not A11 unfortunately
	// make official if successful

	__builtin_write_RPCON(0x0000); // unlock PPS

//    RPINR45bits.CLCINAR = 0x003B;    //RC11->CLC1:CLCINA
    RPINR45bits.CLCINAR = 0x003C;    //RC12->CLC1:CLCINA

    __builtin_write_RPCON(0x0800); // lock PPS


	// same as before
	coreInit();
	setupTitle();
	setupDisk();
	handleFirmwareUpdate();
	updateInit();
	runTitle();
	runFrameLoop();
	
	/*
    9,217    4800          000000       main2      NOP                                           
    9,218    4802          000000                  NOP                                           
    9,219    4804          20FFF0                  MOV #0xFFF, W0                                
    9,220    4806          887260                  MOV W0, CNEN1C                                
    9,221    4808          23F000                  MOV #0x3F00, W0                               
    9,222    480A          887210                  MOV W0, CNPUC                                 
    9,223    480C          07EAEB                  RCALL coreInit                                
    9,224    480E          07EA67                  RCALL setupTitle                              
    9,225    4810          07EF3F                  RCALL setupDisk                               
    9,226    4812          07EFC6                  RCALL handleFirmwareUpdate                    
    9,227    4814          07F09C                  RCALL updateInit                              
    9,228    4816          07EAC0                  RCALL runTitle                                
    9,229    4818          37EF89                  BRA runFrameLoop                              
	*/

#endif
