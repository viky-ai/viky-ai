namespace :statistics do

  desc 'Creates and configures stats indices'
  task setup: :environment do |t, args|
    client = IndexManager.client
    template_config_dir = "#{Rails.root}/config/statistics"
    Dir.foreach(template_config_dir) do |filename|
      next unless filename.downcase.end_with? '.json'
      template_body = JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result)
      template_name = "template-#{template_body['template'][0..-3]}"
      begin
        client.indices.put_template name: template_name, body: template_body
        puts Rainbow("Creation of index template #{template_name} succeed.").green
      rescue
        puts Rainbow("Creation of index template #{template_name} failed.").red
      end
    end
  end
end
