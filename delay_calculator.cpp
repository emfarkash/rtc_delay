#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

struct Stats
{
    int ip_proto;
    int ip_src;
    int tp_src;
    int ip_dst;
    int ssrc;
    int rtp_ts;
    int max_ts_s;
    int max_ts_us;
    int media_type;
    int wall_rtp;
    int ms_tv;
    int star0rtp;
    int star0tv;
    int absolute;
};

bool IsNotVideo(const Stats &stat)
{
    return stat.media_type != 16;
}

bool AreStatsEqual(const Stats &a, const Stats &b)
{
    return a.ip_proto == b.ip_proto && a.ip_src == b.ip_src &&
           a.tp_src == b.tp_src && a.ip_dst == b.ip_dst &&
           a.ssrc == b.ssrc;
}

bool CompareStatsByRtpTs(const Stats &a, const Stats &b)
{
    return a.rtp_ts < b.rtp_ts;
}

int rtp_ts_to_wallclock_ms(int rtp_ts, int sampling_rate_khz)
{
    return static_cast<int>((static_cast<double>(rtp_ts) / static_cast<double>(sampling_rate_khz) * 1000));
}

int timeval_to_ms(int tv_sec, int tv_usec)
{
    return static_cast<int>((static_cast<double>(tv_sec) * 1000.0) + (static_cast<double>(tv_usec) / 1000.0));
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    const char *filePath = argv[1]; // The file path is the first command-line argument

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Error opening the file" << std::endl;
        return 1;
    }
    std::vector<Stats> stats;
    std::string line;
    std::getline(file, line); // Read and discard the header line

    while (std::getline(file, line))
    {
        Stats stat;
        if (sscanf(line.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d",
                   &stat.ip_proto, &stat.ip_src, &stat.tp_src, &stat.ip_dst,
                   &stat.ssrc, &stat.rtp_ts, &stat.max_ts_s, &stat.max_ts_us,
                   &stat.media_type) == 9)
        {
            stats.push_back(stat);
        }
    }
    file.close();

    // Usage in the remove_if
    stats.erase(
        std::remove_if(stats.begin(), stats.end(), IsNotVideo),
        stats.end());

    // Remove duplicate rows
    stats.erase(
        std::unique(stats.begin(), stats.end(), AreStatsEqual),
        stats.end());

    // Sort by RTP timestamp
    std::sort(stats.begin(), stats.end(), CompareStatsByRtpTs);

    // Define constants
    int sampling_rate_khz = 90000;

    // Functions to convert RTP timestamp and Max Time Variable to Wallclock (in ms)

    // Convert RTP timestamp and Max Time Variable to Wallclock (in ms)
    for (auto &stat : stats)
    {
        stat.wall_rtp = rtp_ts_to_wallclock_ms(stat.rtp_ts, sampling_rate_khz);
        stat.ms_tv = timeval_to_ms(stat.max_ts_s, stat.max_ts_us);
    }

    // Start at 0 for RTP timestamp and Max Time Variable to Wallclock (in ms)
    int rtp_first_average_100 = 0;
    int tv_first_average_100 = 0;
    for (int i = 0; i < 100; i++)
    {
        rtp_first_average_100 += stats[i].wall_rtp;
        tv_first_average_100 += stats[i].ms_tv;
    }
    rtp_first_average_100 /= 100;
    tv_first_average_100 /= 100;

    for (auto &stat : stats)
    {
        stat.star0rtp = stat.wall_rtp - rtp_first_average_100;
        stat.star0tv = stat.ms_tv - tv_first_average_100;
    }
    for (size_t i = 0; i < stats.size(); i++)
    {
        stats[i].absolute = stats[i].star0tv - stats[i].star0rtp;
    }

    // Print mean and median
    int sum_absolute = 0;
    std::vector<int> absolute_values;
    for (const Stats &stat : stats)
    {
        sum_absolute += stat.absolute;
        absolute_values.push_back(stat.absolute);
    }

    double mean_absolute = static_cast<double>(sum_absolute) / stats.size();

    // Calculate the median
    std::sort(absolute_values.begin(), absolute_values.end());
    double median_absolute;
    if (absolute_values.size() % 2 == 0)
    {
        median_absolute = (static_cast<double>(absolute_values[absolute_values.size() / 2 - 1]) +
                           static_cast<double>(absolute_values[absolute_values.size() / 2])) /
                          2.0;
    }
    else
    {
        median_absolute = static_cast<double>(absolute_values[absolute_values.size() / 2]);
    }

    std::cout << "Mean: " << mean_absolute << std::endl;
    std::cout << "Median: " << median_absolute << std::endl;

    // Now, 'stats' vector contains the processed data
    return 0;
}
