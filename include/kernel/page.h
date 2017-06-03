#ifndef KERNEL_PAGE_H
#define KERNEL_PAGE_H


#include <config/config.h>


#if defined(CONFIG_KERNEL_VIRT_MEM)

#include <kernel/page_virt.h>

#else // CONFIG_KERNEL_VIRT_MEM

#include <kernel/page_phys.h>

#endif // CONFIG_KERNEL_VIRT_MEM


#endif // KERNEL_PAGE_H
