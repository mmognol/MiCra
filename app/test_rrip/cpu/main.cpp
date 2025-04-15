extern "C"
{
#include <dpu.h>
}

#include <vector>
#include <string>

int main()
{
    dpu_set_t dpus{};
    std::string filename = "dpu/add_in_vecteur.dpu";
    DPU_ASSERT(dpu_alloc_ranks(1, NULL, &dpus));
    DPU_ASSERT(dpu_load(dpus, filename.c_str(), NULL));

    DPU_ASSERT(dpu_launch(dpus, DPU_SYNCHRONOUS));
    
    std::vector<uint64_t> hits(16);
    std::vector<uint64_t> misses(16);
    std::vector<uint64_t> perfcount(16);

    std::vector<uint64_t> arr(500000);

    dpu_set_t dpu{};
    uint64_t each_dpu = 0;
    DPU_FOREACH(dpus, dpu, each_dpu)
    {
        if(each_dpu == 0)
        {
            DPU_ASSERT(dpu_prepare_xfer(dpu, &arr[0]));
            DPU_ASSERT(dpu_push_xfer(dpu, DPU_XFER_FROM_DPU, "arrays", 0, 500000*sizeof(uint64_t), DPU_XFER_ASYNC));


            DPU_ASSERT(dpu_prepare_xfer(dpu, &hits[0]));
            DPU_ASSERT(dpu_push_xfer(dpu, DPU_XFER_FROM_DPU, "hits", 0, 16*sizeof(uint64_t), DPU_XFER_ASYNC));


            DPU_ASSERT(dpu_prepare_xfer(dpu, &misses[0]));
            DPU_ASSERT(dpu_push_xfer(dpu, DPU_XFER_FROM_DPU, "misses", 0, 16*sizeof(uint64_t), DPU_XFER_ASYNC));

            DPU_ASSERT(dpu_prepare_xfer(dpu, &perfcount[0]));
            DPU_ASSERT(dpu_push_xfer(dpu, DPU_XFER_FROM_DPU, "perfcount", 0, 16*sizeof(uint64_t), DPU_XFER_ASYNC));
        }
    }

    dpu_sync(dpus);
    dpu_free(dpus);

    for(uint32_t i = 0; i < 16; ++i)
    {
        printf("Hit on DPU %u: %lu\n", i, hits[i]);
        printf("Miss on DPU %u: %lu\n", i, misses[i]);
    }


    printf("arr[0]: %lu\n", arr[0]);
    printf("arr[1]: %lu\n", arr[1]);
    printf("arr[2]: %lu\n", arr[2]);
    printf("arr[3]: %lu\n", arr[3]);
    printf("arr[4]: %lu\n", arr[4]);
    printf("arr[5]: %lu\n", arr[5]);
    printf("arr[6]: %lu\n", arr[6]);
    printf("arr[7]: %lu\n", arr[7]);

    for(uint32_t i = 0; i < 16; ++i)
    {
        printf("Perfcount on DPU %u: %lu\n", i, perfcount[i]);
    }


    return 0;
}