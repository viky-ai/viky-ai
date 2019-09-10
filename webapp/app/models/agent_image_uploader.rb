require 'image_processing/mini_magick'

class AgentImageUploader < Shrine
  include ImageProcessing::MiniMagick

  plugin :determine_mime_type
  plugin :validation_helpers
  plugin :remove_attachment
  plugin :delete_raw
  plugin :remove_invalid
  plugin :processing
  plugin :versions
  plugin :pretty_location

  Attacher.validate do
    validate_max_size 2.megabytes, message: 'is too large (max is 2 MB)'
    validate_mime_type_inclusion ['image/jpeg', 'image/png', 'image/gif']
  end

  process(:store) do |io, _context|
    pipeline = ImageProcessing::MiniMagick.source(io.download)
    {
      original: io,
      background: pipeline.resize_to_fill!(640, 360, gravity: 'Center'),
      avatar: pipeline.resize_to_fill!(96, 96, gravity: 'Center')
    }
  end
end
