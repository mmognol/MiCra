extern "C"
{
#include <dpu.h>
}

#include <chrono>
#include <vector>
#include <string>


std::vector<uint64_t> expected_result()
{
    std::vector<uint64_t> result(500000);
    for(uint32_t i = 0; i < 500000; i+=4)
    {
        result[i] = i/4 + 1;
        result[i+1] = i/4 + 1;
        result[i+2] = i/4 + 1;
        result[i+3] = i/4 + 8;
    }

    return result;
}

bool check_result(const std::vector<uint64_t>& arr)
{
    auto expected = expected_result();
    for(uint32_t i = 0; i < arr.size(); ++i)
    {
        if(arr[i] != expected[i])
        {
            printf("Error at index %u: expected %lu, got %lu\n", i, expected[i], arr[i]);
            return false;
        }
    }
    return true;
}


int main()
{
    dpu_set_t dpus{};
    std::string filename = "dpu/add_in_vecteur.dpu";
    DPU_ASSERT(dpu_alloc_ranks(1, NULL, &dpus));
    DPU_ASSERT(dpu_load(dpus, filename.c_str(), NULL));


    auto start = std::chrono::high_resolution_clock::now();
    DPU_ASSERT(dpu_launch(dpus, DPU_SYNCHRONOUS));
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    printf("DPU run time: %.6f seconds\n\n", elapsed.count());
    
    std::vector<uint64_t> hits(16);
    std::vector<uint64_t> misses(16);
    std::vector<uint64_t> perfcount(16);
    std::vector<uint32_t> ltouch(16*8);

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

            DPU_ASSERT(dpu_prepare_xfer(dpu, &ltouch[0]));
            DPU_ASSERT(dpu_push_xfer(dpu, DPU_XFER_FROM_DPU, "ltouch", 0, 16*8*sizeof(uint32_t), DPU_XFER_ASYNC));
        }
    }

    dpu_sync(dpus);
    dpu_free(dpus);

    for(uint32_t i = 0; i < 1; ++i)
    {
        printf("Hit on DPU %u: %lu\n", i, hits[i]);
        printf("Miss on DPU %u: %lu\n", i, misses[i]);
    }
    printf("\n");


    if(!check_result(arr))
    {
        printf("Error: result is not correct\n");
        return 1;
    }
    else
    {
        printf("Result is correct\n\n");
    }

    for(uint32_t i = 0; i < 1; ++i)
    {
        printf("Perfcount on DPU %u: %lu\n", i, perfcount[i]);
    }
    
    for(uint32_t i = 0; i < 1; ++i)
    {
        printf("Line touch on DPU %u:  ", i);
        for(uint32_t j = 0; j < 8; ++j)
        {
            printf("(%u: %u) ", j, ltouch[i*8+j]);
        }
        printf("\n");
    }

    return 0;
}