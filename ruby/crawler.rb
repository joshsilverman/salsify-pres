require 'asin'
require 'asin/adapter'
require 'pry'
require 'json'

ASIN::Configuration.configure do |config|
  config.secret        = ENV['AWS_SECRET_KEY']
  config.key           = ENV['AWS_ACCESS_KEY']
  config.associate_tag = 'joshsilv-20'
end

keywords = ['T-Shirt', 'basketball', 'wrench', 'hammer', 'pants', 'gloves', 'jacket', 'pant', 'jean', 'raincoat', 'izod', 'hoodie', 'fleece', 'pant', 'flannel', 'columbia', 'sweater', 'boardshort', 'jogger', 'belt', 'dockers', 'khaki', 'tommy hilfiger', 'tartan', 'Levi\'s', 'button down', 'collar', 'slim fit', 'disney', 'star wars', 'new england patriots', 'harvard', 'woven', 'dickies', 'timberland', 'cargo short', 'long-sleeve', 'axist', 'perry ellis', 'volcom', 'polo', 'nylon', 'twill', 'tee', 'haggar', 'alex stevens', 'HUF', 'calvin klein', 'southpole'].shuffle
cursor_limit = 500
cursor = 0
request_delay = 10
items = []

client = ASIN::Client.instance
keywords.each do |keyword|
  items += client.search_keywords(keyword, :SearchIndex => :All)
  sleep Random.new.rand(request_delay)
end

while !items.empty? do
  item = items.shift

  filename = "../data/products/#{item['asin']}.json"
  if File.exists?(filename) and !File.zero?(filename)
  else
    File.open(filename, 'w+') do |f|
      f.write(item.to_json)
    end

    if cursor < cursor_limit
      items += client.similar(item['asin'])
      sleep Random.new.rand(request_delay)
      cursor += 1
    end
  end
end
