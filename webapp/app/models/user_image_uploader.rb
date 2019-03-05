require "image_processing/mini_magick"

class UserImageUploader < Shrine
  include ImageProcessing::MiniMagick

  plugin :determine_mime_type
  plugin :validation_helpers
  plugin :remove_attachment
  plugin :delete_raw
  plugin :remove_invalid
  plugin :processing
  plugin :versions
  plugin :pretty_location
  plugin :default_url

  Attacher.default_url do
    ActionController::Base.helpers.asset_path("default_avatar.png")
  end

  Attacher.validate do
    validate_max_size 2.megabytes, message: 'is too large (max is 2 MB)'
    validate_mime_type_inclusion ['image/jpeg', 'image/png', 'image/gif']
  end

  process(:store) do |io, _context|
    pipeline = ImageProcessing::MiniMagick.source(io.download)
    {
      original: io,
      square: pipeline.resize_to_fill!(600, 600, gravity: 'Center')
    }
  end

end
