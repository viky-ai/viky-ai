if ENV['COVERAGE']
  SimpleCov.start do
    add_filter "/test/"
    add_filter "/config/"
    add_filter "/app/middlewares/"
    add_filter "/lib/"

    add_group "Controllers", "app/controllers"
    add_group "Channels",    "app/channels/"
    add_group "Models",      "app/models"
    add_group "Mailers",     "app/mailers"
    add_group "Helpers",     "app/helpers"
    add_group "Jobs",        "app/jobs"

    track_files "app/**/*.rb"
  end
end
