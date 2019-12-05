require 'image_processing/mini_magick'

class AgentImageUploader < Shrine
  include ImageProcessing::MiniMagick

  plugin :remove_attachment
  plugin :remove_invalid

  Attacher.validate do
    validate_max_size 2.megabytes, message: 'is too large (max is 2 MB)'
    validate_mime_type_inclusion ['image/jpeg', 'image/png', 'image/gif']
  end

  Attacher.derivatives do |original|
    magick = ImageProcessing::MiniMagick.source(original)
    {
      background: magick.resize_to_fill!(640, 360, gravity: 'Center'),
      avatar: magick.resize_to_fill!(96, 96, gravity: 'Center')
    }
  end
end
