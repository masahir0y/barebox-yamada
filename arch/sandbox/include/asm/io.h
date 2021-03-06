#ifndef __ASM_SANDBOX_IO_H
#define __ASM_SANDBOX_IO_H

#define	IO_SPACE_LIMIT 0

#include <asm-generic/io.h>
#include <asm-generic/bitio.h>

static inline void *phys_to_virt(unsigned long phys)
{
       return (void *)phys;
}

static inline unsigned long virt_to_phys(volatile void *mem)
{
       return (unsigned long)mem;
}

#endif /* __ASM_SANDBOX_IO_H */
