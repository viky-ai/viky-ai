require "shrine"
require "shrine/storage/file_system"

Shrine.storages = {
  cache: Shrine::Storage::FileSystem.new("public", prefix: "uploads/cache"), # temporary
  store: Shrine::Storage::FileSystem.new("public", prefix: "uploads/store"), # permanent
}

Shrine.plugin :activerecord
Shrine.plugin :cached_attachment_data
Shrine.plugin :restore_cached_data
Shrine.plugin :derivatives, versions_compatibility: true
Shrine.plugin :validation_helpers
Shrine.plugin :instrumentation
Shrine.plugin :determine_mime_type
Shrine.plugin :pretty_location

Shrine.logger = Rails.logger

class Shrine::Attacher
  def promote(*)
    create_derivatives
    super
  end
end
