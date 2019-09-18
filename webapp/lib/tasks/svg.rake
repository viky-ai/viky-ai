require_relative 'lib/task'

namespace :svg do

  desc "Update svg icon helper"
  task :icon => [:environment] do |t, args|
    helper_path = File.join(Rails.root, "app", "helpers", "icon_helper.rb")
    File.open(helper_path, 'w') do |helper|
      helper.write "module IconHelper\n\n"
      path = File.join(Rails.root, "app", "assets", "images", "icons", "*.svg")
      Dir.glob(path).each do |icon|
        svg = File.read(icon)
        name = File.basename(icon, ".svg")
        clean_svg = svg.delete("\n").delete("\r").delete("\t")
        helper.write "  def icon_#{name}\n"
        helper.write "    '#{clean_svg}'.html_safe\n"
        helper.write "  end\n\n"
      end
      helper.write "end"
    end
  end

  desc "Update svg logo helper"
  task :logo => [:environment] do |t, args|
    helper_path = File.join(Rails.root, "app", "helpers", "logo_helper.rb")
    File.open(helper_path, 'w') do |helper|
      helper.write "module LogoHelper\n\n"
      path = File.join(Rails.root, "app", "assets", "images", "logo", "*.svg")
      Dir.glob(path).each do |icon|
        svg = File.read(icon)
        name = File.basename(icon, ".svg")
        clean_svg = svg.delete("\n").delete("\r").delete("\t")
        helper.write "  def logo_#{name}\n"
        helper.write "    '#{clean_svg}'.html_safe\n"
        helper.write "  end\n\n"
      end
      helper.write "end"
    end
  end

end
