class AddRightsToMembership < ActiveRecord::Migration[5.1]
  def up
    add_column :memberships, :rights, :string, default: 'show'
    ActiveRecord::Base.record_timestamps = false
    begin
      Membership.all.each do |m|
        m.rights = 'all'
        m.save!
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end
  end

  def down
    remove_column :memberships, :rights
  end
end
