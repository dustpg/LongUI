$filelist = []
def traverse_dir(file_path, ext = '.c')
  if File.directory? file_path
    Dir.foreach(file_path) { |file|
      if file !="." and file !=".."
        traverse_dir(file_path+"/"+file, ext)
      end
    }
  else
    basename = File.basename(file_path)
    if (basename.include?(ext))
      $filelist << File.expand_path(file_path)
    end
  end
end

work_dir = ARGV[0] + "/src"
return if !work_dir
traverse_dir work_dir
traverse_dir work_dir + "/../RichED"
$inclist = {}
$filelist_c = []
$filelist_cpp = []

$filelist.each { |x| 
  if x.end_with? '.c'
    $filelist_c << x
  else
    $filelist_cpp << x
  end
}
$filelist = []
traverse_dir work_dir + "/private/", '.h'
$private_list = $filelist

# $filelist_c.each { |x| p x }
# C FILE
File.open("lui_c.c", 'wb:utf-8') { |file|
  file.write("\xEF\xBB\xBF")
  marco = ARGV[0] + "/include/util/ui_unimacro.h"
  text = File.open(marco, mode: 'r:bom|utf-8').read
  text.each_line { |line|
    if (line.include? '#pragma once')

    else
      file.write line
    end
  }

  $filelist_c.each { |x|
    text = File.open(x, mode: 'r:bom|utf-8').read
    file.write "\n"
    text.each_line { |line|
      if (line.include? 'ui_unimacro.h')

      else
        file.write line
      end
    }
  }
}

# C++ FILE
File.open("lui.cpp", 'wb:utf-8') { |file|
  file.write("\xEF\xBB\xBF")
  file.write("#define _CRT_SECURE_NO_WARNINGS\n")
  file.write("#define NOMINMAX\n")
  file.write("#include \"lui.h\"\n")
  file.write("#include \"lui_os.h\"\n")
  inc_dir = ARGV[0] + "/include"
  
  $private_list.each { |private_header_name| 
    file.write "\n"
    text = File.open(private_header_name, mode: 'r:bom|utf-8').read
    text.each_line { |line|
      if (line.include? '#pragma once')
      elsif (line.include? '#include')
        if line.include?('/') || line.include?('luiconf')
          
        else
          file.write line
        end
      else
        file.write line
      end
    }
  }

  $filelist_cpp.each { |x|
    text = File.open(x, mode: 'r:bom|utf-8').read
    file.write "\n"
    text.each_line { |line|
      this_line = line
      if (line.include? "#include")
        if line.start_with? '//'
          this_line = nil
        else
          inc = nil
          # luiconf private
          if line.include?('luiconf') || line.include?('private')
            this_line = nil
          # <file>
          elsif this_line.include? '<'
            this_line =~ /.+<(.+)>/
            inc = File.expand_path($1, inc_dir)
          # "file"
          else
            this_line =~ /.+"(.+)"/
            inc = File.expand_path($1, File.dirname(x))
          end
          this_line = nil if inc && File.exist?(inc)
        end
      end
      file.write(this_line) if this_line
    }
  }
}
