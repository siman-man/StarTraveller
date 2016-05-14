require 'open3'

FILE_NAME = "StarTraveller"
SEED = 2

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

  1001.upto(1100) do |num|
    file.puts("----- !BEGIN! ------")
    file.puts("Seed = #{seed}")

    data = Open3.capture3("time java -jar visualizer.jar -seed #{seed} -novis -exec './#{FILE_NAME}'")
    file.puts(data.select{|d| d.is_a?(String) }.flat_map{|d| d.split("\n") })
    file.puts("----- !END! ------")
  end

  system('ruby scripts/analyze.rb 100')
end

task :final do
  system('rm result.txt')
  Rake::Task['compile'].invoke

  2001.upto(3000) do |num|
    file.puts("----- !BEGIN! ------")
    file.puts("Seed = #{seed}")

    data = Open3.capture3("time java -jar visualizer.jar -seed #{seed} -novis -exec './#{FILE_NAME}'")
    file.puts(data.select{|d| d.is_a?(String) }.flat_map{|d| d.split("\n") })
    file.puts("----- !END! ------")
  end

  system('ruby scripts/analyze.rb 1000')
end

