#include <array>
#include <ctime>
#include <climits>
#include <filesystem>
#include <random>
#include <unordered_set>
#include <vector>

extern "C"
{
#include <dpu.h>
}


constexpr int64_t bucket_size = 524288; // 2^19;
constexpr int64_t num_dpu_bucket = 16;

/**
 * @brief Initializes a vector of arrays with random integer values.
 *
 * This function takes a vector of arrays as input and initializes each element of the array with a random integer value.
 * The random values are generated using a uniform distribution between 0 and INT32_MAX.
 *
 * @param V The vector of arrays to be initialized.
 */
void initializeRandomValues(std::vector<std::array<int32_t, bucket_size>> &V)
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int32_t> dist(0, INT32_MAX);

#pragma omp parallel for schedule(dynamic)
    for (auto &arr : V)
    {
        for (auto &e : arr)
        {
            e = dist(eng);
        }
    }
}

/**
 * @brief Initializes a DPU set and loads a DPU program from a file.
 *
 * This function initializes a DPU set and loads a DPU program from the specified file.
 * If the file does not exist, it exits the program with an error message.
 *
 * @param filename The path to the DPU program file.
 * @return The initialized DPU set.
 */
dpu_set_t init_dpus(const std::filesystem::path &filename, uint32_t num_dpu)
{
    dpu_set_t dpu{};
    if (!std::filesystem::exists(filename))
        throw std::runtime_error("File " + filename.string() + " not found.");

    DPU_ASSERT(dpu_alloc(num_dpu, nullptr, &dpu));
    DPU_ASSERT(dpu_load(dpu, filename.c_str(), nullptr));
    return dpu;
}

dpu_set_t init_ranks(const std::filesystem::path &filename, size_t num_ranks)
{
    dpu_set_t dpu{};
    if (!std::filesystem::exists(filename))
        throw std::runtime_error("File " + filename.string() + " not found.");

    DPU_ASSERT(dpu_alloc_ranks(num_ranks, nullptr, &dpu));
    DPU_ASSERT(dpu_load(dpu, filename.c_str(), nullptr));
    return dpu;
}

void send_to_dpu(dpu_set_t dpus, std::vector<std::array<int32_t, bucket_size>> &V)
{
    dpu_set_t dpu{};
    uint32_t j = 0;

    for (size_t i = 0; i < num_dpu_bucket; i++)
    {
        DPU_FOREACH(dpus, dpu, j)
        {
            DPU_ASSERT(dpu_prepare_xfer(dpu, V[(j * num_dpu_bucket) + i].data()));
        }
        DPU_ASSERT(dpu_push_xfer(dpus, DPU_XFER_TO_DPU, "g_arr", i * sizeof(int32_t) * bucket_size,
                                 sizeof(int32_t) * bucket_size, DPU_XFER_ASYNC));
    }
}

void gather_from_dpu(dpu_set_t dpus, std::vector<std::array<int32_t, bucket_size>> &V)
{
    dpu_set_t dpu{};
    uint32_t j = 0;

    for (size_t i = 0; i < num_dpu_bucket; i++)
    {
        DPU_FOREACH(dpus, dpu, j)
        {
            DPU_ASSERT(dpu_prepare_xfer(dpu, V[(j * num_dpu_bucket) + i].data()));
        }
        DPU_ASSERT(dpu_push_xfer(dpus, DPU_XFER_FROM_DPU, "g_arr", i * sizeof(int32_t) * bucket_size,
                                 sizeof(int32_t) * bucket_size, DPU_XFER_ASYNC));
    }
}

void launch_dpu(dpu_set_t dpus)
{
    DPU_ASSERT(dpu_launch(dpus, DPU_ASYNCHRONOUS));
}

void test_if_sorted(std::vector<std::array<int32_t, bucket_size>> &V)
{
    size_t count_unsorted = 0;
    for (auto &arr : V)
    {
        for (size_t i = 1; i < arr.size(); i++)
        {
            if (arr[i] < arr[i - 1])
            {
                count_unsorted++;
                break;
            }
        }
    }
    printf("Array unsorted: %3lu/%3lu\n", count_unsorted, V.size());
}

std::filesystem::path parse_dpu_filename(int argc, char *argv[])
{
    if (argc < 2)
    {
        throw std::invalid_argument("Usage: <program> <dpu_filename>");
    }

    std::filesystem::path dpu_path{argv[1]};

    const std::unordered_set<std::string> allowed_filenames = {
        "countsort.dpu", "heapsort.dpu", "quicksort.dpu"};

    if (allowed_filenames.find(dpu_path.filename().string()) == allowed_filenames.end())
    {
        throw std::invalid_argument("Invalid filename. Allowed filenames are: countsort.dpu, heapsort.dpu, quicksort.dpu");
    }

    if (!std::filesystem::exists(dpu_path))
    {
        throw std::runtime_error("File does not exist: " + dpu_path.string());
    }

    return dpu_path;
}

int main(int argc, char *argv[])
{
    auto dpu_filename = parse_dpu_filename(argc, argv);
    auto dpus = init_ranks(dpu_filename, 40);

    uint32_t num_dpu{};
    dpu_get_nr_dpus(dpus, &num_dpu);
    printf("Number of DPUs: %d\n", num_dpu);

    std::vector<std::array<int32_t, bucket_size>> V(num_dpu * num_dpu_bucket);

    auto start_init = std::chrono::high_resolution_clock::now();
    initializeRandomValues(V);
    auto end_init = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_init = end_init - start_init;
    printf("Initialization time: %.6f seconds\n", elapsed_init.count());


    test_if_sorted(V);

    auto start = std::chrono::high_resolution_clock::now();

    send_to_dpu(dpus, V);
    launch_dpu(dpus);
    gather_from_dpu(dpus, V);
    dpu_sync(dpus);
    dpu_free(dpus);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    printf("Elapsed time: %.6f seconds\n", elapsed.count());

    test_if_sorted(V);

    return 0;
}