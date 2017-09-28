class EnablePgcryptoExtension < ActiveRecord::Migration[5.1]
  def change
    enable_extension 'pgcrypto' unless extension_enabled?('pgcrypto')
  end
end
