$filelist = []
def traverse_dir(file_path)
  if File.directory? file_path
    Dir.foreach(file_path) { |file|
      if file !="." and file !=".."
        traverse_dir(file_path+"/"+file)
      end
    }
  else
    basename = File.basename(file_path)
    if (basename != 'luiconf.h' && basename.include?('.h'))
      $filelist << File.expand_path(file_path)
    end
  end
end

work_dir = ARGV[0] + "/include"
return if !work_dir
traverse_dir work_dir
traverse_dir work_dir + "/../TextBC"
$inclist = {}

$is_os = {}

$filelist.each { |filename|
  list = []
  is_os = false
  File.readlines(filename).each { |line|
    if line.start_with? '#include'
      inc = ""
      if line.include? '<'
        line =~ /.+<(.+)>/
        inc = File.expand_path($1, work_dir)
      else
        line =~ /.+"(.+)"/
        inc = File.expand_path($1, File.dirname(filename))
      end

      if !File.exist?(inc)
        if inc.include?('.h')
          is_os = true
          p [inc, filename] 
        end
      elsif (!inc.include? 'luiconf.h')
        list << inc.to_sym
      end
    end
  }
  $is_os[filename.to_sym] = is_os
  # p filename if list.empty?
  $inclist[filename.to_sym] = list
}

$sorted = []

def do_sort(key, values)
  # 已经存在就算了
  return if $sorted.include? key
  p key if values == nil
  # VALUE为空即表示本项目不需要依赖
  values.each { |value|
    do_sort(value, $inclist[value])
  }
  $sorted << key
end

$inclist.each { |key, value| do_sort(key, value) }

$sorted_main = []
$sorted_os = []
$sorted.each { |i|
  if $is_os[i]
    $sorted_os << i
  else
    $sorted_main << i
  end
}
p $sorted_os

def write_file out, list
  work_dir = ARGV[0] + "/include"
  File.open(out, 'wb:utf-8') { |file|
    file.write("\xEF\xBB\xBF")
    file.write("#pragma once\n")
    file.write("#include \"luiconf.h\"\n")
    file.write("#include \"lui.h\"\n") if !out.include? 'lui.h'
    list.each { |sym|
      file.write("\n")
      text = File.open(sym.to_s, mode: 'r:bom|utf-8').read

      text.each_line { |line|
        this_line = line
        if this_line.start_with? '#include'
          inc = ""
          if this_line.include? '<'
            this_line =~ /.+<(.+)>/
            inc = File.expand_path($1, work_dir)
          else
            this_line =~ /.+"(.+)"/
            inc = File.expand_path($1, File.dirname(sym.to_s))
          end

          if File.exist?(inc)
            this_line = nil
          end

        elsif this_line.include? '#pragma once'
          this_line = nil
        end
        file.write(line) if (this_line)
      }
    }
  }
end

write_file "lui.h", $sorted_main
write_file "lui_os.h", $sorted_os