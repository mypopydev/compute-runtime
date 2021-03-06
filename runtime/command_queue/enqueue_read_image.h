/*
 * Copyright (C) 2017-2018 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "runtime/built_ins/built_ins.h"
#include "hw_cmds.h"
#include "runtime/command_queue/command_queue_hw.h"
#include "runtime/command_stream/command_stream_receiver.h"
#include "runtime/event/event.h"
#include "runtime/helpers/surface_formats.h"
#include "runtime/helpers/cache_policy.h"
#include "runtime/helpers/kernel_commands.h"
#include "runtime/helpers/basic_math.h"
#include "runtime/helpers/mipmap.h"
#include "runtime/mem_obj/image.h"
#include "runtime/memory_manager/surface.h"
#include <algorithm>
#include <new>

namespace OCLRT {

template <typename GfxFamily>
cl_int CommandQueueHw<GfxFamily>::enqueueReadImage(
    Image *srcImage,
    cl_bool blockingRead,
    const size_t *origin,
    const size_t *region,
    size_t inputRowPitch,
    size_t inputSlicePitch,
    void *ptr,
    cl_uint numEventsInWaitList,
    const cl_event *eventWaitList,
    cl_event *event) {

    notifyEnqueueReadImage(srcImage, !!blockingRead);

    MultiDispatchInfo di;
    auto isMemTransferNeeded = true;
    if (srcImage->isMemObjZeroCopy()) {
        size_t hostOffset;
        Image::calculateHostPtrOffset(&hostOffset, origin, region, inputRowPitch, inputSlicePitch, srcImage->getImageDesc().image_type, srcImage->getSurfaceFormatInfo().ImageElementSizeInBytes);
        isMemTransferNeeded = srcImage->checkIfMemoryTransferIsRequired(hostOffset, 0, ptr, CL_COMMAND_READ_IMAGE);
    }
    if (!isMemTransferNeeded) {
        NullSurface s;
        Surface *surfaces[] = {&s};
        enqueueHandler<CL_COMMAND_MARKER>(
            surfaces,
            blockingRead == CL_TRUE,
            di,
            numEventsInWaitList,
            eventWaitList,
            event);
        if (event) {
            auto pEvent = castToObjectOrAbort<Event>(*event);
            pEvent->setCmdType(CL_COMMAND_READ_IMAGE);
        }

        if (context->isProvidingPerformanceHints()) {
            context->providePerformanceHint(CL_CONTEXT_DIAGNOSTICS_LEVEL_GOOD_INTEL, CL_ENQUEUE_READ_IMAGE_DOESNT_REQUIRES_COPY_DATA, static_cast<cl_mem>(srcImage));
        }

        return CL_SUCCESS;
    }

    auto &builder = getDevice().getExecutionEnvironment()->getBuiltIns()->getBuiltinDispatchInfoBuilder(EBuiltInOps::CopyImage3dToBuffer,
                                                                                                        this->getContext(), this->getDevice());

    BuiltInOwnershipWrapper builtInLock(builder, this->context);

    size_t hostPtrSize = calculateHostPtrSizeForImage(const_cast<size_t *>(region), inputRowPitch, inputSlicePitch, srcImage);
    void *dstPtr = ptr;

    MemObjSurface srcImgSurf(srcImage);
    HostPtrSurface hostPtrSurf(dstPtr, hostPtrSize);
    Surface *surfaces[] = {&srcImgSurf, &hostPtrSurf};

    if (region[0] != 0 &&
        region[1] != 0 &&
        region[2] != 0) {
        bool status = getDevice().getCommandStreamReceiver().createAllocationForHostSurface(hostPtrSurf, getDevice());
        if (!status) {
            return CL_OUT_OF_RESOURCES;
        }
        dstPtr = reinterpret_cast<void *>(hostPtrSurf.getAllocation()->getGpuAddressToPatch());
    }

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    dc.srcMemObj = srcImage;
    dc.dstPtr = dstPtr;
    dc.srcOffset = origin;
    dc.size = region;
    dc.srcRowPitch = inputRowPitch;
    dc.srcSlicePitch = inputSlicePitch;
    if (srcImage->getImageDesc().num_mip_levels > 0) {
        dc.srcMipLevel = findMipLevel(srcImage->getImageDesc().image_type, origin);
    }
    builder.buildDispatchInfos(di, dc);

    enqueueHandler<CL_COMMAND_READ_IMAGE>(
        surfaces,
        blockingRead == CL_TRUE,
        di,
        numEventsInWaitList,
        eventWaitList,
        event);

    if (context->isProvidingPerformanceHints()) {
        if (!isL3Capable(ptr, hostPtrSize)) {
            context->providePerformanceHint(CL_CONTEXT_DIAGNOSTICS_LEVEL_BAD_INTEL, CL_ENQUEUE_READ_IMAGE_DOESNT_MEET_ALIGNMENT_RESTRICTIONS, ptr, hostPtrSize, MemoryConstants::pageSize, MemoryConstants::pageSize);
        }
    }

    return CL_SUCCESS;
}
} // namespace OCLRT
