#
#  @file Makefile
#
#  @brief Tangxun building script
#
#  @author Min Zhang
#
#  @note
#

TOPDIR := $(shell /bin/pwd)
TODAY_DATE := $(shell date +%Y%m%d)

# build directory
build_dir = ./tangxun
build_dir_bin = $(build_dir)/bin
build_dir_lib = $(build_dir)/lib
build_dir_config = $(build_dir)/config
build_dir_script = $(build_dir)/script
build_dir_db = $(build_dir)/db

# Tangxun software
tangxun_sw_src_dir = $(TOPDIR)/../tangxun_sw

# Tangxun software bin files
tangxun_sw_bin_dir = $(tangxun_sw_src_dir)/build/bin
tangxun_sw_dest_bin_dir = $(build_dir_bin)

# Tangxun software lib files
tangxun_sw_lib_dir = $(tangxun_sw_src_dir)/build/lib
tangxun_sw_dest_lib_dir = $(build_dir_lib)

# Tangxun software config files
tangxun_sw_config_dir = $(tangxun_sw_src_dir)/config
tangxun_sw_config_files = servmgmt.setting StockList.txt
tangxun_sw_config_sector_dir = $(tangxun_sw_config_dir)/sector
tangxun_sw_dest_config_dir = $(build_dir_config)

# Tangxun software db files
tangxun_sw_db_dir = $(tangxun_sw_src_dir)/db
tangxun_sw_db_files = tangxun.db trade.db
tangxun_sw_dest_db_dir = $(build_dir_db)

# Tangxun software jtk file
tangxun_sw_jtk_src_dir = $(TOPDIR)/../tangxun_sw/jtk
tangxun_sw_jtk_lib_dir = $(tangxun_sw_jtk_src_dir)/lib
tangxun_sw_jtk_bin_dir = $(tangxun_sw_jtk_src_dir)/bin
tangxun_sw_jtk_bin_files = jf_genuuid jf_dongyuan jf_servmgmt
tangxun_sw_jtk_dest_bin_dir = $(build_dir_bin)
tangxun_sw_jtk_dest_lib_dir = $(build_dir_lib)

# image parameter
# image size in MB
image_size = 50
image_name = tangxunimage
image_loop_dev = /dev/loop6
image_mount_point = imagefsdir
image_crypt_name = tangxunimagecrypt

CP = /bin/cp
INSTALL = /usr/bin/install

all: build_dir build_3rdparty sw
	@echo "###### Completed building all ######"

build_3rdparty: #.build_xxx
	@echo "###### Completed building 3rd party ######"

build_dir:
	@echo "###### Create build dir ######"
	@mkdir -p $(build_dir)
	@mkdir -p $(build_dir_bin)
	@mkdir -p $(build_dir_lib)
	@mkdir -p $(build_dir_config)
	@mkdir -p $(build_dir_script)
	@mkdir -p $(build_dir_db)

sw: sw_build copy_jtk copy_scripts copy_config copy_db
	@echo "###### Completed building tangxun sofware ######"

copy_scripts:
	@for i in $(script_files); do \
        cp -af $(script_src_dir)/$$i $(build_dir_script); \
    done
	@echo "###### Completed copying Tangxun software script files ######"

copy_config:
	@for i in $(tangxun_sw_config_files); do \
        cp -af $(tangxun_sw_config_dir)/$$i $(build_dir_config); \
    done
	@cp -afr $(tangxun_sw_config_sector_dir) $(build_dir_config);
	@echo "###### Completed copying Tangxun software config files ######"

copy_db:
	@for i in $(tangxun_sw_db_files); do \
        cp -af $(tangxun_sw_db_dir)/$$i $(build_dir_db); \
    done
	@echo "###### Completed copying Tangxun software db files ######"

copy_jtk:
	@cp -af $(tangxun_sw_jtk_lib_dir)/* $(build_dir_lib)
	@for i in $(tangxun_sw_jtk_bin_files); do \
        cp -af $(tangxun_sw_jtk_bin_dir)/$$i $(build_dir_bin); \
    done
	@echo "###### Completed copying jtk ######"

sw_build: sw_dirs .build_sw
	@for i in $(tangxun_sw_bin_dir)/*; do \
        cp -af $$i $(build_dir_bin); \
    done
	@for i in $(tangxun_sw_lib_dir)/*; do \
        cp -af $$i $(build_dir_lib); \
    done
	@echo "###### Completed coping Tangxun software ######"

sw_dirs:
	@echo "###### Completed creating Tangxun software dir ######"

.build_sw:
	@echo "###### Building Tangxun software ######"
	@(cd $(tangxun_sw_src_dir); make -f linux.mak)
#	@touch .build_sw
	@echo "###### Completed making Tangxun software ######"

sw_clean:
	@(cd $(tangxun_sw_src_dir); make -f linux.mak clean)
	@rm -f .build_sw
	@echo "----- Cleaned Tangxun software -----"

image:
	@echo "----- Start making image -----"
	dd if=/dev/urandom of=$(image_name) bs=1M count=$(image_size)
	losetup $(image_loop_dev) $(image_name)
	cryptsetup -y luksFormat $(image_loop_dev)
	cryptsetup luksOpen $(image_loop_dev) $(image_crypt_name)
	mkfs.ext3 /dev/mapper/$(image_crypt_name)
	tune2fs -c 0 -i 0 /dev/mapper/$(image_crypt_name)
	if [ ! -d $(image_mount_point) ]; then mkdir -p $(image_mount_point); fi
	mount /dev/mapper/$(image_crypt_name) $(image_mount_point)
	cp -ar $(build_dir) $(image_mount_point)
	umount $(image_mount_point)
	cryptsetup luksClose $(image_crypt_name)
	losetup -d $(image_loop_dev)

cleanimage:
	@echo "----- Clean image -----"
	if [ -d $(image_mount_point) ]; then \
         umount $(image_mount_point); \
         rm -fr $(image_mount_point); \
    fi
	if [ -e /dev/mapper/$(image_crypt_name) ]; then cryptsetup luksClose $(image_crypt_name); fi
	losetup -d $(image_loop_dev)
	rm -f $(image_name)

clean:
	@$(MAKE) sw_clean
	@$(RM) -rf $(build_dir)


