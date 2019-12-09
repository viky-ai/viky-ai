require "image_processing/mini_magick"

class UserImageUploader < Shrine
  include ImageProcessing::MiniMagick

  plugin :remove_attachment
  plugin :remove_invalid
  plugin :default_url

  Attacher.default_url do
    ActionController::Base.helpers.asset_path("default_avatar.png")
  end

  Attacher.validate do
    validate_max_size 2.megabytes, message: 'is too large (max is 2 MB)'
    validate_mime_type_inclusion ['image/jpeg', 'image/png', 'image/gif']
  end

  Attacher.derivatives do |original|
    magick = ImageProcessing::MiniMagick.source(original)
    {
      square: magick.resize_to_fill!(600, 600, gravity: 'Center')
    }
  end

end
