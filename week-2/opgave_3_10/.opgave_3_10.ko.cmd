cmd_/home/bedirhan/linux-kernel-development/week-2/opgave_3_10/opgave_3_10.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /home/bedirhan/linux-kernel-development/week-2/opgave_3_10/opgave_3_10.ko /home/bedirhan/linux-kernel-development/week-2/opgave_3_10/opgave_3_10.o /home/bedirhan/linux-kernel-development/week-2/opgave_3_10/opgave_3_10.mod.o ;  true