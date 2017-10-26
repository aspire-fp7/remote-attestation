-include $(BLOCKS_INCLUDE_FILE)


C_FLAGS := -std=c99 
ROOT = $(shell dirname $(shell pwd))

OUTDIR := ./maketh

MANAGER_OUTPUT				:=	$(OUTDIR)/manager_exe
VERIFIER_OUTPUT				:=	$(OUTDIR)/verifier_exe
VERIFIER_FORWARDER_OUTPUT 	:=	$(OUTDIR)/verifier_forwarder_exe
EXTRACTOR_OUTPUT			:=	$(OUTDIR)/extractor_exe


THIRD_PARTY_DIR				=	/opt/3rd_party
THIRD_PARTY_INCLUDE			=	-I$(THIRD_PARTY_DIR)/libwebsockets/serverlinux/include \
								-I$(THIRD_PARTY_DIR)/openssl/serverlinux/include \

ASCL_INCLUDE				=	-I/opt/ASCL/include

THIRD_PARTY_LIB				=	-L$(THIRD_PARTY_DIR)/libwebsockets/serverlinux/lib \
								-L$(THIRD_PARTY_DIR)/openssl/serverlinux/lib \

ifdef DBG
	DBG_FLAGS := -DDEBUG -g3
else
	DBG_FLAGS :=
endif

ifdef HDBG
	DBG_FLAGS := -DDEBUG -DHARD_DEBUG -g3
endif
	

CC = gcc
LD = ld

RM := rm -rfv
CP := cp -rfv


################################################################
#		Definition of directories							   #
################################################################
BLOCKS_OBJS_DIR 			:= $(OUTDIR)/blocks
SUPPORT_STUFF_OBJS_DIR 		:= $(OUTDIR)/support_stuff
ATTESTATOR_OBJS_DIR 		:= $(OUTDIR)/attester
MANAGER_OBJS_DIR 			:= $(OUTDIR)/manager
VERIFIER_OBJS_DIR 			:= $(OUTDIR)/verifier
VERIFIER_FORWARDER_OBJS_DIR	:= $(OUTDIR)/verifier_forwarder
EXTRACTOR_OBJS_DIR 			:= $(OUTDIR)/extractor
BLAKE2_OBJS_DIR 			:= $(OUTDIR)/blake2

HEADERS_DIR				:= $(SRCDIR)/headers 
HEADERS_PARAM_DIR		:= $(SRCDIR)/client-parametric-files/uncond
C_FLAGS += -I$(HEADERS_DIR) -I$(HEADERS_PARAM_DIR)

OBJS_DIRS = \
	$(BLOCKS_OBJS_DIR) \
	$(SUPPORT_STUFF_OBJS_DIR) \
	$(MANAGER_OBJS_DIR) \
	$(ATTESTATOR_OBJS_DIR) \
	$(VERIFIER_OBJS_DIR) \
	$(VERIFIER_FORWARDER_OBJS_DIR) \
	$(EXTRACTOR_OBJS_DIR) \
	$(BLAKE2_OBJS_DIR)

#################################################################
#		Definition of FUNDAMENTAL BLOCKS names and objects 		#
#################################################################


#RA_DATA_PREPARATION_BLOCK		= 	$(BLOCKS_OBJS_DIR)/$(RA_DATA_PREPARATION_BLOCK_NAME).o
#RA_DATA_TABLE_BLOCK				=	$(BLOCKS_OBJS_DIR)/$(RA_DATA_TABLE_BLOCK_NAME).o
#RA_DO_HASH_BLOCK				=	$(BLOCKS_OBJS_DIR)/$(RA_DO_HASH_BLOCK_NAME).o
#RA_MEMORY_BLOCK					=	$(BLOCKS_OBJS_DIR)/$(RA_MEMORY_BLOCK_NAME).o
#RA_NONCE_GENERATION_BLOCK		=	$(BLOCKS_OBJS_DIR)/$(RA_NONCE_GENERATION_BLOCK_NAME).o
#RA_NONCE_INTERPRETATION_BLOCK	=	$(BLOCKS_OBJS_DIR)/$(RA_NONCE_INTERPRETATION_BLOCK_NAME).o

RA_DATA_PREPARATION_BLOCK		= 	$(BLOCKS_OBJS_DIR)/$(RA_DATA_PREPARATION_BLOCK_NAME).o
RA_DATA_TABLE_BLOCK				=	$(BLOCKS_OBJS_DIR)/$(RA_DATA_TABLE_BLOCK_NAME).o
RA_DO_HASH_BLOCK				=	$(BLOCKS_OBJS_DIR)/$(RA_DO_HASH_BLOCK_NAME).o
RA_MEMORY_BLOCK					=	$(BLOCKS_OBJS_DIR)/$(RA_MEMORY_BLOCK_NAME).o
RA_NONCE_GENERATION_BLOCK		=	$(BLOCKS_OBJS_DIR)/$(RA_NONCE_GENERATION_BLOCK_NAME).o
RA_NONCE_INTERPRETATION_BLOCK	=	$(BLOCKS_OBJS_DIR)/$(RA_NONCE_INTERPRETATION_BLOCK_NAME).o



##################################################################


#################################################################
#		Definition of BLOCKS objects dependencies for each 		#
#		infrastructure element									#
#################################################################

# Application indipendent
MANAGER_BLOCKS_OBJS = 

VERIFIER_BLOCKS_OBJS = \
	$(RA_DO_HASH_BLOCK)

VERIFIER_FORWARDER_BLOCKS_OBJS = 

EXTRACTOR_BLOCKS_OBJS = \
	$(RA_DATA_PREPARATION_BLOCK) \
	$(RA_DATA_TABLE_BLOCK) \
	$(RA_DO_HASH_BLOCK) \
	$(RA_MEMORY_BLOCK) \
	$(RA_NONCE_GENERATION_BLOCK) \
	$(RA_NONCE_INTERPRETATION_BLOCK)
################################################################




################################################################
#		Definition of COMPONENTS objects					   #
################################################################
MANAGER_OBJS = \
	$(MANAGER_OBJS_DIR)/manager.o

VERIFIER_OBJS = \
	$(VERIFIER_OBJS_DIR)/verifier.o 

VERIFIER_FORWARDER_OBJS = \
	$(VERIFIER_FORWARDER_OBJS_DIR)/verifier_forwarder.o 

EXTRACTOR_OBJS = \
	$(EXTRACTOR_OBJS_DIR)/extractor.o

################################################################


################################################################
#		Definition of INCLUDES objects dependencies for each   #
#		infrastructure element								   #
################################################################
MANAGER_INCLUDES_OBJS = \
	$(SUPPORT_STUFF_OBJS_DIR)/ra_database.o \
	$(SUPPORT_STUFF_OBJS_DIR)/ra_client.o \
	$(SUPPORT_STUFF_OBJS_DIR)/generic_list.o \
	$(SUPPORT_STUFF_OBJS_DIR)/scheduler_queue.o

VERIFIER_INCLUDES_OBJS = \
	$(SUPPORT_STUFF_OBJS_DIR)/ra_database.o 
		
ATTESTATOR_INCLUDES_OBJS = \

EXTRACTOR_INCLUDES_OBJS = \
	$(SUPPORT_STUFF_OBJS_DIR)/ra_database.o \
	$(SUPPORT_STUFF_OBJS_DIR)/generic_functions.o 
################################################################


################################################################
#	Definition of dependency objects for each final target	   #
################################################################
ATTESTATOR_FINAL_OBJS = \
	$(ATTESTATOR_BLOCKS_OBJS) \
	$(ATTESTATOR_OBJS) \
	$(ATTESTATOR_INCLUDES_OBJS) \
#	$(ACCL_OBJ) 

MANAGER_FINAL_OBJS = \
	$(MANAGER_BLOCKS_OBJS) \
	$(MANAGER_OBJS) \
	$(MANAGER_INCLUDES_OBJS) \
	$(ASCL)
	#TODO remove ascl.o after ACTC integration

VERIFIER_FORWARDER_FINAL_OBJS = \
	$(VERIFIER_FORWARDER_OBJS)

VERIFIER_FINAL_OBJS = \
	$(VERIFIER_INCLUDES_OBJS) \
	$(VERIFIER_BLOCKS_OBJS) \
	$(VERIFIER_OBJS)

EXTRACTOR_FINAL_OBJS = \
	$(EXTRACTOR_OBJS) \
	$(EXTRACTOR_BLOCKS_OBJS) \
	$(EXTRACTOR_INCLUDES_OBJS)

ifeq '$(RA_DO_HASH_BLOCK_NAME)' 'ra_do_hash_blake2'
        VERIFIER_FINAL_OBJS += $(BLAKE2_OBJS_DIR)/blake2s.o
        EXTRACTOR_FINAL_OBJS += $(BLAKE2_OBJS_DIR)/blake2s.o
endif

################################################################


###############################################################

TO_BE_CLEANED = $(OBJS_DIRS) \
                $(VERIFIER_OUTPUT) \
                $(EXTRACTOR_OUTPUT) \
                $(VERIFIER_FORWARDER_OUTPUT)\
                $(MANAGER_OUTPUT)
################################################################


# All Target
all: all-indipendent all-specific
all-clean: all-indipendent all-specific clean-partials
all-indipendent:  manager verifier-forwarder 
all-specific: verifier extractor 
all-indipendent-clean:  manager verifier-forwarder clean-partials
all-specific-clean: verifier extractor clean-partials

$(OUTDIR)/blake2/%.o: $(SRCDIR)/ra-additionals/blake2s-ref.c
	@echo 'Building file: $<'
	mkdir -p $(BLAKE2_OBJS_DIR)
	$(CC) -c "$<" -o "$@" $(C_FLAGS) 
	
$(OUTDIR)/blocks/%.o: $(SRCDIR)/client-parametric-files/cond/%.c
	@echo 'Building file: $<'
	mkdir -p $(BLOCKS_OBJS_DIR)
	$(CC) $(C_FLAGS) $(DBG_FLAGS) $(THIRD_PARTY_INCLUDE) -c "$<" -o "$@"

$(OUTDIR)/support_stuff/%.o: $(SRCDIR)/ra-additionals/%.c
	@echo 'Building file: $<'	
	mkdir -p $(SUPPORT_STUFF_OBJS_DIR)
	$(CC) $(C_FLAGS) $(DBG_FLAGS) $(THIRD_PARTY_INCLUDE) -c "$<" -o "$@"

$(OUTDIR)/manager/%.o: $(SRCDIR)/server-components/%.c
	@echo 'Building file: $<'
	@mkdir -pv $(MANAGER_OBJS_DIR)
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(C_FLAGS) $(DBG_FLAGS) $(THIRD_PARTY_INCLUDE) $(ASCL_INCLUDE) -c "$<" -o "$@" 

$(OUTDIR)/verifier/%.o: $(SRCDIR)/server-components/%.c
	@echo 'Building file: $<'
	mkdir -p $(VERIFIER_OBJS_DIR)
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(C_FLAGS) $(DBG_FLAGS) $(THIRD_PARTY_INCLUDE) -c "$<" -o "$@" 

$(OUTDIR)/verifier_forwarder/%.o: $(SRCDIR)/server-components/%.c
	@echo 'Building file: $<'
	mkdir -p $(VERIFIER_FORWARDER_OBJS_DIR)
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(C_FLAGS) $(DBG_FLAGS) -c "$<" -o "$@" 

$(OUTDIR)/extractor/%.o: $(SRCDIR)/server-components/%.c
	@echo 'Building file: $<'
	mkdir -p $(EXTRACTOR_OBJS_DIR)
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(C_FLAGS) $(DBG_FLAGS) $(THIRD_PARTY_INCLUDE) -c "$<" -o "$@"




manager: $(MANAGER_FINAL_OBJS)
	@echo 'Linking: $@'	
	$(CC) $(MANAGER_FINAL_OBJS) -o $(MANAGER_OUTPUT) $(THIRD_PARTY_LIB) -static -lmysqlclient -ldl -lwebsockets -lssl -lcrypto -pthread -lm -lz
	@echo 'Finished building $@'
	@echo "REMEMBER: this element is application indpendent (it's enough to generate it just once)."
	@echo ' '

verifier-forwarder: $(VERIFIER_FORWARDER_FINAL_OBJS)
	@echo 'Linking: $@'
	@echo 'Invoking: GCC C Linker'
	$(CC) $(VERIFIER_FORWARDER_FINAL_OBJS) -o $(VERIFIER_FORWARDER_OUTPUT) 
	@echo 'Finished building $@'
	@echo ' '

verifier: $(VERIFIER_FINAL_OBJS)
	@echo 'Linking: $@'
	@echo 'Invoking: GCC C Linker'
	$(CC) $(VERIFIER_FINAL_OBJS) -o $(VERIFIER_OUTPUT) $(THIRD_PARTY_LIB)  -lmysqlclient -lcrypto 
	@echo 'Finished building $@'
	@echo ' '

extractor: $(EXTRACTOR_FINAL_OBJS)
	@echo 'Linking: $@'
	@echo 'Invoking: GCC C Linker'
	$(CC) $(EXTRACTOR_FINAL_OBJS) -o $(EXTRACTOR_OUTPUT) $(THIRD_PARTY_LIB) -lmysqlclient -lcrypto -ldl -lbfd -lm
	@echo 'Finished building $@'
	@echo ' '


# Other Targets
clean:
	@echo "Total cleaning, removing all generated artifacts"
	@$(RM) $(TO_BE_CLEANED)
	@echo ' '	

clean-partials:
	@echo "Cleaning partial artifacts, removing only .o files"
	@$(RM) $(OBJS_DIRS)
	@echo ' '	

.PHONY: all clean manager verifier-forwarder verifier extractor





