require 'open3'

FILE_NAME = "StarTraveller"
SEED = 1037

desc 'c++ file compile'
task :default do
  system("g++ -std=c++11 -W -Wall -Wno-sign-compare -O2 -o #{FILE_NAME} #{FILE_NAME}.cpp")
end

desc 'c++ file compile'
task :compile do
  system("g++ -std=c++11 -W -Wall -Wno-sign-compare -O2 -o #{FILE_NAME} #{FILE_NAME}.cpp")
end

desc 'exec and view result'
task :run do
  Rake::Task['compile'].invoke
  system("java -jar ./visualizer.jar -vis -seed #{SEED} -exec './#{FILE_NAME}'")
end

desc 'check single'
task :one do
  Rake::Task['compile'].invoke
  #system("time java -jar StarTraveller.jar -save result.png -seed #{SEED} -novis -exec './#{FILE_NAME}'")
  system("time java -jar visualizer.jar -save result.png -seed #{SEED} -novis -exec './#{FILE_NAME}'")
end

desc 'check for windows'
task :windows do
  Rake::Task['compile'].invoke
  system("java -jar ./visualizer.jar -novis -seed #{SEED} -exec ./#{FILE_NAME}.exe")
end

desc 'check out of memory'
task :debug do
  system("g++ -std=c++11 -W -Wall -g -fsanitize=address -fno-omit-frame-pointer -Wno-sign-compare -O2 -o #{FILE_NAME} #{FILE_NAME}.cpp")
  system("time java -jar visualizer.jar -seed #{SEED} -novis -exec './#{FILE_NAME}'")
end

desc 'check how many called each function'
task :coverage do
  system("g++ -W -Wall -Wno-sign-compare -o #{FILE_NAME} --coverage #{FILE_NAME}.cpp")
  system("time java -jar visualizer.jar -seed #{SEED} -novis -exec './#{FILE_NAME}'")
end

desc 'clean file'
task :clean do
  system("rm data/*.*")
  system("rm *.gcda")
  system("rm *.gcov")
  system("rm *.gcno")
end

desc 'sample'
task :sample do
  system('rm result.txt')
  Rake::Task['compile'].invoke

  File.open('result.txt', 'w') do |file|
    1.upto(10) do |seed|
      puts "seed = #{seed}"
      file.puts("----- !BEGIN! ------")
      file.puts("Seed = #{seed}")

      data = Open3.capture3("time java -jar visualizer.jar -seed #{seed} -novis -exec './#{FILE_NAME}'")
      file.puts(data.select{|d| d.is_a?(String) }.flat_map{|d| d.split("\n") })
      file.puts("----- !END! ------")
    end
  end

  system("ruby scripts/analyze.rb 10")
end

task :test do
  system('rm result.txt')
  Rake::Task['compile'].invoke

  File.open('result.txt', 'w') do |file|
    1001.upto(1100) do |seed|
      puts "seed = #{seed}"
      file.puts("----- !BEGIN! ------")
      file.puts("Seed = #{seed}")

      data = Open3.capture3("time java -jar visualizer.jar -seed #{seed} -novis -exec './#{FILE_NAME}'")
      file.puts(data.select{|d| d.is_a?(String) }.flat_map{|d| d.split("\n") })
      file.puts("----- !END! ------")
    end
  end

  system('ruby scripts/analyze.rb 100')
end

task :final do
  system('rm result.txt')
  Rake::Task['compile'].invoke

  File.open('result.txt', 'w') do |file|
    2001.upto(3000) do |seed|
      puts "seed = #{seed}"
      file.puts("----- !BEGIN! ------")
      file.puts("Seed = #{seed}")

      data = Open3.capture3("time java -jar visualizer.jar -seed #{seed} -novis -exec './#{FILE_NAME}'")
      file.puts(data.select{|d| d.is_a?(String) }.flat_map{|d| d.split("\n") })
      file.puts("----- !END! ------")
    end
  end

  system('ruby scripts/analyze.rb 1000')
end

task :select do
  system('rm result.txt')
  Rake::Task['compile'].invoke

  #data = [2010, 2014, 2023, 2025, 2032, 2035, 2045, 2047, 2050, 2051, 2056, 2063, 2068, 2084, 2091, 2092, 2104, 2112, 2114, 2116, 2120, 2139, 2141, 2146, 2165, 2166, 2169, 2174, 2193, 2220, 2224, 2229, 2231, 2240, 2243, 2245, 2248, 2249, 2267, 2271, 2284, 2285, 2287, 2288, 2296, 2304, 2311, 2313, 2319, 2323, 2325, 2326, 2328, 2344, 2355, 2374, 2377, 2379, 2380, 2393, 2400, 2422, 2423, 2430, 2432, 2436, 2437, 2439, 2446, 2468, 2469, 2479, 2480, 2496, 2499, 2501, 2506, 2514, 2515, 2521, 2525, 2545, 2547, 2553, 2571, 2578, 2582, 2584, 2594, 2602, 2603, 2606, 2614, 2623, 2630, 2635, 2637, 2641, 2654, 2660, 2676, 2681, 2684, 2686, 2704, 2711, 2721, 2729, 2733, 2739, 2740, 2741, 2750, 2763, 2771, 2778, 2780, 2782, 2790, 2792, 2794, 2809, 2818, 2822, 2830, 2833, 2841, 2843, 2853, 2867, 2884, 2888, 2889, 2921, 2930, 2938, 2949, 2950, 2957, 2959, 2983, 2986]
  #data = [2025, 2051, 2063, 2091, 2120, 2139, 2165, 2224, 2231, 2243, 2245, 2313, 2325, 2326, 2393, 2432, 2468, 2499, 2501, 2545, 2571, 2582, 2602, 2603, 2614, 2623, 2635, 2637, 2654, 2686, 2780, 2794, 2822, 2841, 2957, 2986]
  #data = [2002, 2003, 2006, 2015, 2049, 2063, 2078, 2087, 2108, 2140, 2185, 2187, 2199, 2201, 2205, 2222, 2224, 2239, 2241, 2251, 2269, 2280, 2339, 2342, 2365, 2389, 2392, 2401, 2413, 2432, 2451, 2477, 2478, 2485, 2503, 2505, 2529, 2531, 2540, 2557, 2565, 2571, 2586, 2590, 2593, 2598, 2602, 2603, 2613, 2617, 2618, 2628, 2650, 2653, 2654, 2685, 2686, 2693, 2712, 2719, 2728, 2751, 2774, 2777, 2783, 2786, 2794, 2798, 2804, 2806, 2874, 2875, 2882, 2913, 2974, 2978, 2982, 2986]
  data = [2002, 2003, 2006, 2015, 2049, 2063, 2078, 2087, 2108]
  #data = [2018, 2027, 2041, 2044, 2051, 2057, 2058, 2060, 2091, 2093, 2113]
  #data = [2018, 2027, 2041, 2044, 2051, 2057, 2058, 2060, 2091, 2093, 2113, 2120, 2121, 2128, 2139, 2142, 2147, 2158, 2171, 2181, 2217, 2219, 2231, 2245, 2275, 2276, 2290, 2293, 2301, 2313, 2324, 2325, 2326, 2337, 2340, 2354, 2362, 2385, 2393, 2411, 2415, 2418, 2419, 2441, 2456, 2463, 2465, 2468, 2474, 2483, 2498, 2499, 2501, 2504, 2508, 2512, 2524, 2545, 2549, 2563, 2568, 2607, 2614, 2673, 2701, 2720, 2748, 2769, 2803, 2805, 2807, 2815, 2816, 2817, 2848, 2871, 2880, 2886, 2892, 2894, 2904, 2929, 2957, 2962, 2964, 2981]
  #data = [2012, 2025, 2040, 2061, 2069, 2080, 2089, 2115, 2118, 2134, 2136, 2143, 2145, 2165, 2189, 2190, 2198, 2208, 2209, 2233, 2242, 2243, 2254, 2294, 2307, 2347, 2351, 2368, 2373, 2378, 2402, 2403, 2421, 2435, 2459, 2460, 2470, 2492, 2494, 2502, 2535, 2546, 2552, 2555, 2564, 2570, 2573, 2581, 2582, 2587, 2591, 2609, 2620, 2623, 2635, 2637, 2645, 2648, 2696, 2708, 2709, 2736, 2742, 2761, 2780, 2799, 2822, 2836, 2839, 2841, 2881, 2883, 2935, 2951, 2960, 2963, 2965, 2969, 2984, 2990]
  puts data.size

  File.open('result.txt', 'w') do |file|
    data.each do |seed|
      puts "seed = #{seed}"
      file.puts("----- !BEGIN! ------")
      file.puts("Seed = #{seed}")

      data = Open3.capture3("time java -jar visualizer.jar -seed #{seed} -novis -exec './#{FILE_NAME}'")
      file.puts(data.select{|d| d.is_a?(String) }.flat_map{|d| d.split("\n") })
      file.puts("----- !END! ------")
    end
  end

  system('ruby scripts/analyze.rb 100')
end

