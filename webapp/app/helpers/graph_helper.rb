module GraphHelper

  def prepare_requests_data(processed_requests, rejected_requests, from, to)
    JSON.generate(
      from: from,
      to: to,
      data: [
      {
        name: 'Processed requests',
        data: processed_requests.map do |requests|
          [requests['key'], requests['doc_count']]
        end
      },
      {
        name: 'Rejected requests',
        data: rejected_requests.map do |requests|
          [requests['key'], requests['doc_count']]
        end
      }
    ]).html_safe
  end

  def prepare_expressions_data(user)
    expressions = user.expressions_count
    quota = Rack::Throttle.expressions_limit
    JSON.generate({
      consumed: quota.nil? ? 0 : (expressions * 100 / quota.to_f).to_i,
      label: "#{expressions} used out of #{quota}"
    }).html_safe
  end
end