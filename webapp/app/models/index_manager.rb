module IndexManager

  def self.template_configuration
    template_config_dir = "#{Rails.root}/config/statistics"
    filename = 'template-stats-interpret_request_log.json'
    JSON.parse(ERB.new(File.read("#{template_config_dir}/#{filename}")).result)
  end
end
