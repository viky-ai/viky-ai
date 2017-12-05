class TableInterpretationAliasColumnAliasnameMustBePresent < ActiveRecord::Migration[5.1]
  def change
    ActiveRecord::Base.record_timestamps = false
    begin
      InterpretationAlias.all.each do |ialias|
        if ialias.aliasname.blank?
          ialias.aliasname = "#{ialias.intent.intentname}_#{ialias.position_start}_#{ialias.position_end}"
          ialias.save!
        end
      end
    ensure
      ActiveRecord::Base.record_timestamps = true
    end

    change_column_null(:interpretation_aliases, :aliasname, false)
  end
end
