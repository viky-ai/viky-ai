require "shrine"
require "shrine/storage/file_system"

Shrine.storages = {
  cache: Shrine::Storage::FileSystem.new(Rails.root.join("public"), prefix: "uploads/cache"), # temporary
  store: Shrine::Storage::FileSystem.new(Rails.root.join("public"), prefix: "uploads/store"), # permanent
}

Shrine.plugin :activerecord
Shrine.plugin :cached_attachment_data
